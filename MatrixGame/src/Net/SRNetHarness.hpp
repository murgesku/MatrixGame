// SR coop port: invisible networking harness for cooperative planetary battles.
// Design: DESIGN-netlayer.md (M1 closed, M2 handshake per HANDOFF.md sec 10).
// With co-op disabled the engine behaves exactly like upstream — the per-tick
// pump is a guarded no-op.
//
// NOTE: all public functions use explicit __cdecl because the engine is built
// with /Gr (fastcall default). Without this, cross-TU calls would mismatch.
#ifndef SRNETHARNESS_HPP_INCLUDED
#define SRNETHARNESS_HPP_INCLUDED

// Startup: reads the co-op switch. Disabled by default; none of these calls
// touch network state unless co-op is enabled. Installs the L3G idle hook
// (pre-battle handshake) only when enabled.
void __cdecl SRNetInit(void);

// Per-tick pump: exchange orders / pose deltas when enabled. No-op when not.
void __cdecl SRNetPump(void);

// Same pump, entry from the L3G idle hook (counted separately, M2).
// L3GNetPumpHook is __fastcall under engine /Gr, so this matches.
void __fastcall SRNetPumpHooked(void);

// Same pump, entry from the CMatrixMap::Takt call site (counted separately).
void __cdecl SRNetPumpTakt(void);

// Shutdown: disconnect and release.
void __cdecl SRNetDeinit(void);

// True when co-op networking is enabled (even if the session never connected).
bool __cdecl SRNetEnabled(void);

// SR coop port (M2): register the map identity we are about to load. The
// harness compares it against the handshake packet; a mismatch refuses the
// session. Safe to call before/around map load; NULL clears it.
void __cdecl SRNetSetMapName(const wchar * map);

// SR coop port (M2 step 3): send pre-built pose pod on channel B (unreliable).
// Caller builds the payload (see SPKTPoseEntry) and calls once per Takt.
void __cdecl SRNetSendPosePod(const void *pod, int size);

// SR coop port (M2 step 3): retrieve received pose entries from the internal
// buffer. Returns count of entries copied into buf (max maxCnt).
int __cdecl SRNetGetPoseBuf(struct SPKTPoseEntry *buf, int maxCnt);

// SR coop port (M2): per-file log name with the SR_LOG_PREFIX environment
// prefix applied ("hst_srnet.log" style) so two engine instances on one
// machine never scribble each other's logs. Returns base unchanged when the
// prefix is unset. Every harness file write must go through this.
const char * __cdecl SRLogName(const char * base);

// True once the M2 handshake completed on both sides (HELLO/WELcome exchanged).
bool __cdecl SRNetSessionUp(void);

// True when co-op networking is running in host mode.
bool __cdecl SRNetIsHost(void);

// Pose packet type on channel B (unreliable, sequenced).
static const unsigned char PKT_POSE = 3;

// Order packet type on channel A (reliable, ordered).
static const unsigned char PKT_ORDER = 4;

// ---- Pose packet format (channel B, unreliable, sequenced) ----
// Payload: header (4 bytes) + entries (24 bytes each). All little-endian,
// byte-packed. Built by the host each Takt from m_Side[side]->m_Robots[].
#pragma pack(push, 1)
struct SPKTPoseEntry
{
    unsigned long unitId;   // index in m_Side[side]->m_Robots[]
    float         x;        // CMatrixRobot::m_PosX
    float         y;        // CMatrixRobot::m_PosY
    float         heading;  // CMatrixRobot::m_Unit[0].m_Angle
    unsigned long phase;    // CMatrixRobotAI order phase (ROP_*)
    float         speed;    // CMatrixRobot::m_Speed
};
#pragma pack(pop)

// ---- Order packet format (channel A, reliable, sequenced) ----
// Payload: header (4 bytes) + entries (24 bytes each). All little-endian,
// byte-packed. Built by the guest each Takt from its side-2 robots' orders.
// m_ObjCore pointer is NOT transmitted (network-invariant); MVP sends only
// scalar order fields.
#pragma pack(push, 1)
struct SPKTOrderEntry
{
    unsigned long unitId;   // index in m_Side[side]->m_Robots[] (owner robot)
    unsigned long orderType;// OrderType  (ROT_MOVE_TO / ROT_FIRE / ...)
    unsigned long orderPhase; // OrderPhase (ROP_MOVING / ROP_FIRING / ...)
    float         param1;     // SOrder::m_Param1
    float         param2;     // SOrder::m_Param2
    float         param3;     // SOrder::m_Param3
    unsigned long param4;     // SOrder::m_Param4
};
#pragma pack(pop)

// SR coop port (M2 step 4): send pre-built order pod on channel A (reliable).
// Caller builds the payload (see SPKTOrderEntry) and calls once per Takt.
void __cdecl SRNetSendOrderPod(const void *pod, int size);

// SR coop port (M2 step 4): log that a pose pod was sent (called from MatrixLogic).
void __cdecl SRNetLogPoseSent(unsigned long count);

// SR coop port (M2 step 4): send pose snapshot from idle hook (called when Takt
// is not available). Built and sent from MatrixLogic.cpp.
void __cdecl SRNetSendPosesFromTick(void);

// SR coop port (M2 step 4): retrieve received order entries from the internal
// host-side buffer. Returns count of entries copied into buf (max maxCnt).
int __cdecl SRNetGetOrderBuf(struct SPKTOrderEntry *buf, int maxCnt);

#endif // SRNETHARNESS_HPP_INCLUDED
