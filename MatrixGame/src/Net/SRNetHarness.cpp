// SR coop port: invisible networking harness, M2 milestone.
//
// M1 (closed): compiles into both targets; fully OFF unless enabled via env;
// host/guest sockets live; per-tick pump counts cycles.
//
// M2 (this file): pre-battle handshake over the L3G idle hook + the reliable
// channel, so the session is established BEFORE the first map Takt:
//   guest -> host: PKT_HELLO   {magic, version, side request, map name}
//   host -> guest: PKT_WELCOME {magic, version, granted side,  map name}
// POD structs, little-endian (both ends are the same Win32 x86 image; make the
// version explicit anyway so a future cross-build has something to assert on).
// Any magic/version/name mismatch -> refuse + disconnect.
//
// Channel B (unreliable, sequenced) is used for pose snapshots:
//   host -> guest: PKT_POSE {type, side, count, entries...}
// The SPKTPoseEntry layout is defined in SRNetHarness.hpp; MatrixLogic.cpp
// builds the host-side packet and parses the guest-side buffer (it has full
// access to engine types, SRNetHarness does not, to avoid winsock2/GROUP
// header conflicts).
//
// Log separation for the two-instances-on-one-machine smoke (HANDOFF sec 10):
// every file this harness writes goes through SRLogName(), which prepends the
// SR_LOG_PREFIX environment value ("hst_srnet.log", "gst_srnet.log", ...).
//
// Switches (environment):
//  SR_COOP        = "host" | "guest"   (any other/absent -> co-op off)
//  SR_COOP_PORT   = server port, default 25333
//  SR_COOP_ADDR   = host address to connect to (guest), default 127.0.0.1
//  SR_COOP_SIDE   = our engine side id, default: host=1, guest=2
//  SR_LOG_PREFIX  = filename prefix for the harness logs (default none)
//
#include <winsock2.h>      // ENet requirement: precedes windows.h so that the
                            // project headers below do not pull legacy winsock.h
                            // (else winsock2 redefined-type errors)
#include "../stdafx.h"

#include "SRNetHarness.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <enet/enet.h>

// ---- protocol constants ----------------------------------------------------//

static const unsigned long SRNET_MAGIC   = 0x534F4352UL;   // 'RCOS'
static const unsigned short SRNET_VER    = 1;

enum SRPacketType
{
    PKT_HELLO   = 1,     // guest -> host, reliable chan 0
    PKT_WELCOME = 2      // host -> guest, reliable chan 0
};

// Channel layout (matches enet_host_create(..., 2, ...) from M1):
// 0 = reliable control (handshake, orders later)
// 1 = unreliable sequenced (pose deltas)
static const enet_uint8 SRChanControl = 0;
static const enet_uint8 SRChanPose   = 1;

// Handshake payload. 1-byte packet type first, then fixed POD. Everything is
// byte-packed (no padding): the structs go onto the wire as-is (ENet packets
// are byte arrays), both ends are built from the same headers anyway.
#pragma pack(push, 1)
struct SPKTHello
{
    unsigned char  type;                      // PKT_HELLO
    unsigned long  magic;                     // SRNET_MAGIC
    unsigned short version;                   // SRNET_VER
    unsigned char  side_request;              // requested engine side
    char           mapname[64];               // base name, 0-padded
};

struct SPKTWelcome
{
    unsigned char  type;                      // PKT_WELCOME
    unsigned long  magic;
    unsigned short version;
    unsigned char  side_granted;              // granted engine side
    char           mapname[64];
};
#pragma pack(pop)

// ---- local state -----------------------------------------------------------//

static bool          s_Enabled     = false;
static bool          s_Host        = false;
static int           s_MySide      = 1;
static int           s_Port        = 25333;
static bool          s_SessionUp   = false;
static bool          s_MapGuessSet = false;
static char          s_MapName[64];           // ANSI version of our map name

static ENetHost     *s_HostIf        = NULL;   // server (host) or client socket
static ENetPeer     *s_GuestPeer     = NULL;   // host mode: the connected guest
static ENetPeer     *s_ServerPeer    = NULL;   // guest mode: connection to host
static unsigned long s_PumpCycles    = 0;
// SR coop port (M2) diagnostics: who drives the pump — the L3G idle hook or
// the per-Takt call site. Logged in SRNetDeinit; small, keep it.
static unsigned long s_PumpFromHook  = 0;
static unsigned long s_PumpFromTakt  = 0;

// SR coop port (M2 step 3): pose receive buffer (guest side).
static SPKTPoseEntry s_PoseBuf[60];            // MAX_ROBOTS
static int           s_PoseBufCnt   = 0;
static bool          s_PoseFirstReceived  = false;

// SR coop port (M2 step 4): order receive buffer (host side).
static SPKTOrderEntry s_OrderBuf[60];          // MAX_ROBOTS
static int           s_OrderBufCnt   = 0;
static bool          s_OrderFirstReceived = false;

// Forward: low-level pod send on a given channel with flags (static, defined below).
static bool SRNetSendPod(ENetPeer * peer, const void * pod, int size,
                         enet_uint8 channel, enet_uint32 flags);

// Forward: log helper (defined below).
static void SRNetLog(const char *msg, unsigned long a, unsigned long b);

// ---- helpers ---------------------------------------------------------------//

static bool SRNetReadEnvSide(void)
{
    char buf[32];
    if (GetEnvironmentVariableA("SR_COOP_SIDE", buf, sizeof(buf)) == 0) return false;
    int v = atoi(buf);
    if (v <= 0 || v > 4) return false;      // engine has 4 play sides (LoadSide, cnt==5 with neutral)
    s_MySide = v;
    return true;
}

const char * __cdecl SRLogName(const char * base)
{
    static char prefixbuf[32] = "";
    static char buf[80];
    if (prefixbuf[0] == 0)
    {
        GetEnvironmentVariableA("SR_LOG_PREFIX", prefixbuf, sizeof(prefixbuf));
    }
    if (prefixbuf[0] == '\0') return base;
    _snprintf(buf, sizeof(buf) - 1, "%s_%s", prefixbuf, base);
    buf[sizeof(buf) - 1] = 0;
    return buf;
}

// Copy the base file name (after the last \ /) into dst (ANSI, truncated).
static void SRNetCopyMapName(char * dst, int csz, const wchar * map)
{
    if (dst == NULL || csz < 1) return;
    dst[0] = '\0';
    if (map == NULL) return;
    const wchar * base = map;
    for (const wchar * p = map; *p != 0; ++p)
        if (*p == L'\\' || *p == L'/') base = p + 1;
    int i = 0;
    for (; base[i] != 0 && i < csz - 1; ++i)
        dst[i] = (char)(unsigned char)base[i];    // ANSI names are ASCII here
    dst[i] = '\0';
}

void __cdecl SRNetSetMapName(const wchar * map)
{
    SRNetCopyMapName(s_MapName, sizeof(s_MapName), map);
    s_MapGuessSet = (s_MapName[0] != '\0');
}

void __cdecl SRNetInit(void)
{
    char buf[64];

    buf[0] = '\0';
    if (GetEnvironmentVariableA("SR_COOP", buf, sizeof(buf)) == 0) return;
    if (_stricmp(buf, "host") == 0)
    {
        s_Host = true;
    } else if (_stricmp(buf, "guest") == 0)
    {
        s_Host = false;
    } else
    {
        return;                                              // unknown value -> stay off
    }

    buf[0] = '\0';
    if (GetEnvironmentVariableA("SR_COOP_PORT", buf, sizeof(buf)) != 0 && buf[0] != '\0')
    {
        int v = atoi(buf);
        if (v > 0 && v < 65536) s_Port = v;
    }

    if (SRNetReadEnvSide() == false) s_MySide = s_Host ? 1 : 2;

    if (enet_initialize() != 0)
    {
        SRNetLog("enet_initialize failed", 0, 0);
        return;
    }

    ENetAddress addr;
    memset(&addr, 0, sizeof(addr));

    if (s_Host)
    {
        addr.host = ENET_HOST_ANY;
        addr.port = (enet_uint16)s_Port;
        s_HostIf = enet_host_create(&addr, 2, 2, 0, 0);      // up to 2 peers (MVP=1), 2 channels
        if (s_HostIf == NULL)
        {
            SRNetLog("host create failed", s_Port, 0);
            enet_deinitialize();
            return;
        }
        SRNetLog("host listening on port", s_Port, 0);
    } else
    {
        s_HostIf = enet_host_create(NULL, 2, 2, 0, 0);
        if (s_HostIf == NULL)
        {
            SRNetLog("guest create failed", 0, 0);
            enet_deinitialize();
            return;
        }
        SRNetLog("guest socket created", s_Port, 0);
    }

    s_Enabled = true;
    SRNetLog("harness enabled, side", (unsigned long)s_MySide, s_Host ? 1 : 0);

    // SR coop port (M2): install the idle hook so the handshake can settle
    // before the first map Takt (L3GRun loop, menu/idle and the DLL battle
    // path alike). Only when enabled — co-op-off keeps the hook NULL.
    extern void (*L3GNetPumpHook)(void);                     // MatrixLib/3G
    L3GNetPumpHook = &SRNetPumpHooked;
}

bool __cdecl SRNetIsHost(void)
{
    return s_Host;
}

bool __cdecl SRNetSessionUp(void)
{
    return s_SessionUp;
}

void __cdecl SRNetSendPosePod(const void *pod, int size)
{
    if (pod == NULL || size < 1 || s_HostIf == NULL || s_GuestPeer == NULL) return;
    ENetPacket * pkt = enet_packet_create(pod, (size_t)size, 0);  // unreliable
    if (pkt == NULL) return;
    if (enet_peer_send(s_GuestPeer, SRChanPose, pkt) < 0)
    {
        enet_packet_destroy(pkt);
        return;
    }
    enet_host_flush(s_HostIf);
    SRNetLog("host: pose pod sent", (unsigned long)size, 0);
}

void __cdecl SRNetLogPoseSent(unsigned long count)
{
    SRNetLog("host: first PKT_POSE sent", count, 1);
}

int __cdecl SRNetGetPoseBuf(SPKTPoseEntry *buf, int maxCnt)
{
    if (buf == NULL || maxCnt < 1 || s_PoseBufCnt <= 0) return 0;
    int n = s_PoseBufCnt < maxCnt ? s_PoseBufCnt : maxCnt;
    memcpy(buf, s_PoseBuf, (size_t)n * sizeof(SPKTPoseEntry));
    return n;
}

void __cdecl SRNetSendOrderPod(const void *pod, int size)
{
    if (pod == NULL || size < 1 || s_HostIf == NULL || s_ServerPeer == NULL) return;
    ENetPacket * pkt = enet_packet_create(pod, (size_t)size, ENET_PACKET_FLAG_RELIABLE);  // reliable
    if (pkt == NULL) return;
    if (enet_peer_send(s_ServerPeer, SRChanControl, pkt) < 0)
    {
        enet_packet_destroy(pkt);
        return;
    }
    enet_host_flush(s_HostIf);
}

int __cdecl SRNetGetOrderBuf(SPKTOrderEntry *buf, int maxCnt)
{
    if (buf == NULL || maxCnt < 1 || s_OrderBufCnt <= 0) return 0;
    int n = s_OrderBufCnt < maxCnt ? s_OrderBufCnt : maxCnt;
    memcpy(buf, s_OrderBuf, (size_t)n * sizeof(SPKTOrderEntry));
    return n;
}

static bool SRNetSendPod(ENetPeer * peer, const void * pod, int size,
                         enet_uint8 channel, enet_uint32 flags)
{
    if (peer == NULL || s_HostIf == NULL || pod == NULL || size < 1) return false;
    ENetPacket * pkt = enet_packet_create(pod, (size_t)size, flags);
    if (pkt == NULL) return false;
    if (enet_peer_send(peer, channel, pkt) < 0)
    {
        enet_packet_destroy(pkt);
        return false;
    }
    enet_host_flush(s_HostIf);
    return true;
}

// ---- helpers ---------------------------------------------------------------//

static void SRNetLog(const char *msg, unsigned long a, unsigned long b)
{
    FILE *f = fopen(SRLogName("srnet.log"), "a");
    if (f == NULL) return;
    fprintf(f, "cycle=%lu %s %lu %lu\n", s_PumpCycles, msg, a, b);
    fclose(f);
}

// ---- M2 handshake ----------------------------------------------------------//

static void SRNetRefusePeer(ENetPeer * peer, const char * why)
{
    SRNetLog(why, 0, 0);
    enet_peer_disconnect_later(peer, 0);
}

static void SRNetHandleHello(ENetEvent * ev)
{
    if (ev->packet->dataLength < sizeof(SPKTHello))
    {
        SRNetRefusePeer(ev->peer, "host: HELLO too short, refusing");
        return;
    }
    SPKTHello hello;
    memcpy(&hello, ev->packet->data, sizeof(SPKTHello));

    if (hello.magic != SRNET_MAGIC || hello.version != SRNET_VER)
    {
        SRNetRefusePeer(ev->peer, "host: HELLO magic/version mismatch, refusing");
        return;
    }
    // Map identity: if we already know our map name, both must match.
    if (s_MapGuessSet && s_MapName[0] != '\0' && hello.mapname[0] != '\0' &&
        _stricmp(hello.mapname, s_MapName) != 0)
    {
        SRNetRefusePeer(ev->peer, "host: HELLO map mismatch, refusing");
        return;
    }
    if (hello.side_request < 1 || hello.side_request > 4)
    {
        SRNetRefusePeer(ev->peer, "host: HELLO bad side, refusing");
        return;
    }

    s_GuestPeer = ev->peer;
    s_SessionUp = true;
    SRNetLog("host: session established with guest side", hello.side_request, 0);

    SPKTWelcome w;
    memset(&w, 0, sizeof(w));
    w.type         = PKT_WELCOME;
    w.magic        = SRNET_MAGIC;
    w.version      = SRNET_VER;
    w.side_granted = (unsigned char)hello.side_request;
    // echo map name back (both ends then assert on the same identity)
    strncpy(w.mapname, s_MapName, sizeof(w.mapname) - 1);
    enet_packet_destroy(ev->packet);

    if (SRNetSendPod(s_GuestPeer, &w, sizeof(w), SRChanControl, ENET_PACKET_FLAG_RELIABLE))
        SRNetLog("host: WELCOME sent", 0, 0);
}

static void SRNetHandleWelcome(ENetEvent * ev)
{
    if (ev->peer != s_ServerPeer)
    {
        enet_packet_destroy(ev->packet);
        return;                                    // stale pre-connect peer
    }
    if (ev->packet->dataLength < sizeof(SPKTWelcome))
    {
        SRNetRefusePeer(ev->peer, "guest: WELCOME too short, dropping");
        return;
    }
    SPKTWelcome w;
    memcpy(&w, ev->packet->data, sizeof(SPKTWelcome));
    enet_packet_destroy(ev->packet);

    if (w.magic != SRNET_MAGIC || w.version != SRNET_VER)
    {
        SRNetRefusePeer(ev->peer, "guest: WELCOME magic/version mismatch, dropping");
        return;
    }
    if (s_MapGuessSet && s_MapName[0] != '\0' && w.mapname[0] != '\0' &&
        _stricmp(w.mapname, s_MapName) != 0)
    {
        SRNetRefusePeer(ev->peer, "guest: WELCOME map mismatch, dropping");
        return;
    }

    // On M2 the granted side is recorded but the simulation still runs
    // unchanged; later milestones bind it to ownership.
    if (w.side_granted >= 1 && w.side_granted <= 4) s_MySide = w.side_granted;

    s_SessionUp = true;
    SRNetLog("guest: session established, granted side", w.side_granted, 0);
}

static void SRNetHandleOrder(ENetEvent * ev)
{
    if (ev->packet->dataLength < (int)(sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned short)))
    {
        enet_packet_destroy(ev->packet);
        return;
    }
    unsigned char *data = ev->packet->data;
    unsigned char  otype = data[0];
    unsigned char  aside = data[1];
    unsigned short ocount;
    memcpy(&ocount, data + 2, sizeof(ocount));
    if (otype != PKT_ORDER || aside < 1 || aside > 4 || ocount > 60 ||
        ev->packet->dataLength < (int)(4 + ocount * sizeof(SPKTOrderEntry)))
    {
        enet_packet_destroy(ev->packet);
        return;
    }
    SPKTOrderEntry *entries = (SPKTOrderEntry *)(data + 4);
    memcpy(s_OrderBuf, entries, ocount * sizeof(SPKTOrderEntry));
    s_OrderBufCnt = ocount;
    if (!s_OrderFirstReceived)
    {
        s_OrderFirstReceived = true;
        SRNetLog("host: first PKT_ORDER received", ocount, aside);
    }
    enet_packet_destroy(ev->packet);
}

static void SRNetGuestEnsureConnected(void)
{
    if (s_ServerPeer != NULL) return;
    if (s_HostIf == NULL) return;

    char addrbuf[64];
    addrbuf[0] = '\0';
    GetEnvironmentVariableA("SR_COOP_ADDR", addrbuf, sizeof(addrbuf));
    if (addrbuf[0] == '\0')
    {
        strcpy(addrbuf, "127.0.0.1");
    }

    ENetAddress addr;
    memset(&addr, 0, sizeof(addr));
    if (enet_address_set_host(&addr, addrbuf) != 0)
    {
        unsigned ip[4] = {0, 0, 0, 0};
        if (sscanf(addrbuf, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]) == 4)
        {
            addr.host = (enet_uint32)(ip[0] | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24));
        } else
        {
            SRNetLog("guest: bad SR_COOP_ADDR", 0, 0);
            return;
        }
    }
    addr.port = (enet_uint16)s_Port;

    s_ServerPeer = enet_host_connect(s_HostIf, &addr, 2, 0);
    if (s_ServerPeer == NULL)
    {
        SRNetLog("guest: connect create failed", 0, 0);
        return;
    }
    SRNetLog("guest: connecting to host", addr.port, 0);
}

static void SRNetGuestSendHello(void)
{
    if (s_ServerPeer == NULL) return;
    SPKTHello hello;
    memset(&hello, 0, sizeof(hello));
    hello.type         = PKT_HELLO;
    hello.magic        = SRNET_MAGIC;
    hello.version      = SRNET_VER;
    hello.side_request = (unsigned char)s_MySide;
    strncpy(hello.mapname, s_MapName, sizeof(hello.mapname) - 1);
    if (SRNetSendPod(s_ServerPeer, &hello, sizeof(hello), SRChanControl, ENET_PACKET_FLAG_RELIABLE))
        SRNetLog("guest: HELLO sent", hello.side_request, 0);
}

// ---- pump ------------------------------------------------------------------//

// Entry from the L3G idle hook (M2): same pump, counted separately.
// L3GNetPumpHook is __fastcall under engine /Gr, so this must match.
void __fastcall SRNetPumpHooked(void)
{
    ++s_PumpFromHook;
    SRNetPump();
    // SR coop port (M2 step 4): send poses from idle hook when Takt() is not
    // being called (e.g. in menu/idle). Ensures pose replication works even
    // when CMatrixMapLogic::Takt() is not driven.
    SRNetSendPosesFromTick();
}

// Entry from the CMatrixMapLogic::Takt call site (M2): counted separately.
void __cdecl SRNetPumpTakt(void)
{
    ++s_PumpFromTakt;
    SRNetPump();
    static bool s_taktLogged = false;
    if (s_SessionUp && s_Host && !s_taktLogged)
    {
        s_taktLogged = true;
        SRNetLog("host: first takt after session up", s_PumpCycles, 0);
    }
}

// SR coop port (M2): the session state machine. Runs pre-battle (from the L3G
// idle hook) and during battle (from CMatrixMap::Takt) alike.
void __cdecl SRNetPump(void)
{
    if (s_Enabled == false) return;

    ++s_PumpCycles;

    if (s_HostIf == NULL) return;

    if (s_Host == false && s_ServerPeer == NULL) SRNetGuestEnsureConnected();

    // Service whatever connection state we are in.
    ENetEvent ev;
    while (enet_host_service(s_HostIf, &ev, 0) > 0)
    {
        switch (ev.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                if (s_Host)
                {
                    s_GuestPeer = ev.peer;               // unconfirmed until HELLO
                    SRNetLog("host: guest connected", 0, 0);
                } else
                {
                    s_ServerPeer = ev.peer;
                    SRNetLog("guest: host session up", 0, 0);
                    SRNetGuestSendHello();               // connect event -> announce us
                }
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // SR coop port (M2): hard guard — a receive event with an
                // out-of-band packet pointer was an AV at connect caused by
                // an ENetEvent layout mismatch (ENet lib vs /Zp1 harness);
                // keep the cheap drop-path in place.
                if (ev.packet == NULL || ev.packet->dataLength < 1)
                {
                    SRNetLog("pump: BAD receive packet, dropping", (unsigned long)ev.channelID, (unsigned long)ev.packet);
                    if (ev.packet != NULL) enet_packet_destroy(ev.packet);
                    break;
                }
                if (ev.channelID == SRChanControl)
                {
                    unsigned char id = ev.packet->data[0];
                    if (s_Host && id == PKT_HELLO   && s_GuestPeer == ev.peer)  SRNetHandleHello(&ev);
                    else if (!s_Host && id == PKT_WELCOME && s_ServerPeer == ev.peer) SRNetHandleWelcome(&ev);
                    else if (s_Host && id == PKT_ORDER && s_GuestPeer == ev.peer) SRNetHandleOrder(&ev);
                    else enet_packet_destroy(ev.packet);
                }
                else if (ev.channelID == SRChanPose)
                {
                    // M2 step 3: receive pose snapshot from host.
                    if (!s_Host && s_SessionUp &&
                        ev.packet->dataLength >= (int)(sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned short)))
                    {
                        unsigned char *data = ev.packet->data;
                        unsigned char  ptype = data[0];
                        unsigned char  pside = data[1];
                        unsigned short pcount;
                        memcpy(&pcount, data + 2, sizeof(pcount));
                        if (ptype == PKT_POSE && pside >= 1 && pside <= 4 && pcount <= 60 &&
                            ev.packet->dataLength >= (int)(4 + pcount * sizeof(SPKTPoseEntry)))
                        {
                            SPKTPoseEntry *entries = (SPKTPoseEntry *)(data + 4);
                            memcpy(s_PoseBuf, entries, pcount * sizeof(SPKTPoseEntry));
                            s_PoseBufCnt = pcount;
                            if (!s_PoseFirstReceived)
                            {
                                s_PoseFirstReceived = true;
                                SRNetLog("guest: first PKT_POSE received", pcount, pside);
                            }
                        }
                    }
                    enet_packet_destroy(ev.packet);
                }
                else
                {
                    // M3: route payload into the order queue / pose applier.
                    enet_packet_destroy(ev.packet);
                }
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                if (ev.peer == s_ServerPeer)  { s_ServerPeer = NULL;  s_SessionUp = false; SRNetLog("guest: host gone", 0, 0); }
                if (ev.peer == s_GuestPeer)   { s_GuestPeer  = NULL;  s_SessionUp = false; SRNetLog("host: guest gone", 0, 0); }
                break;
            default:
                break;
        }
    }
}

void __cdecl SRNetDeinit(void)
{
    if (s_Enabled == false && s_HostIf == NULL) return;

    SRNetLog("harness deinit after cycles", s_PumpCycles, s_SessionUp ? 1 : 0);
    SRNetLog("pump sources: hook", s_PumpFromHook, s_PumpFromTakt);

    extern void (*L3GNetPumpHook)(void);
    L3GNetPumpHook = NULL;

    if (s_GuestPeer  != NULL) enet_peer_disconnect_now(s_GuestPeer, 0);
    if (s_ServerPeer != NULL) enet_peer_disconnect_now(s_ServerPeer, 0);
    if (s_HostIf     != NULL) enet_host_destroy(s_HostIf);
    s_HostIf     = NULL;
    s_GuestPeer  = NULL;
    s_ServerPeer = NULL;
    s_Enabled    = false;
    s_SessionUp  = false;
    s_PumpCycles = 0;

    enet_deinitialize();
}

bool __cdecl SRNetEnabled(void)
{
    return s_Enabled;
}
