#include <cheats.hpp>

#include <MatrixConfig.hpp>
#include <MatrixLogic.hpp>
#include <MatrixRobot.hpp>
#include <MatrixObjectCannon.hpp>
#include <Interface/CConstructor.h>

#include <input.hpp>

#include <deque>
#include <functional>

extern CMatrixMapLogic* g_MatrixMap;
extern DWORD g_Flags;

struct STextInfo {
    const wchar_t* t1;
    const wchar_t* t2;
    uint32_t time;
};

static STextInfo stuff[] = {{L"3D Robots game information....", L"", 3000},
                            {L"Coding....", L"", 10000},
                            {L" Alexander <ZakkeR> Zeberg", L"", 0},
                            {L"", L"Engine lead coder", 0},
                            {L"", L"MapEditor lead coder", 0},
                            {L"", L"Optimizations", 0},
                            {L" Alexey <Dab> Dubovoy", L"", 0},
                            {L"", L"High-AI lead coder", 0},
                            {L"", L"MapEditor base coder", 0},
                            {L" Alexander <Sub0> Parshin", L"", 0},
                            {L"", L"Low-AI lead coder", 0},
                            {L"", L"UI lead coder", 0},

                            {L"Artwork...", L"", 10000},
                            {L" Eugene <Johan> Cherenkov", L"", 0},
                            {L"", L"UI", 0},
                            {L"", L"Some cool textures", 0},
                            {L"", L"Sky", 0},

                            {L" Nina <Nina> Vatulich", L"", 0},
                            {L"", L"Terrain textures", 0},
                            {L"", L"Effects textures", 0},

                            {L"Modeling...", L"", 8000},
                            {L" Nina <Nina> Vatulich", L"", 0},
                            {L"", L"Lot of meshes", 0},

                            {L" Alexander <Alexartist> Yazynin", L"", 0},
                            {L"", L"Meshes", 0},
                            {L"", L"Buildings", 0},

                            {L" Ruslan <IronFist> Tchernyi", L"", 0},
                            {L"", L"Advanced meshes", 0},

                            {L" Sergey <Esk> Simonov", L"", 0},
                            {L"", L"Robots", 0},
                            {L"", L"Helicopters", 0},
                            {L"", L"Some meshes", 0},

                            {L"Map design...", L"", 7000},
                            {L" Alexander <Alexartist> Yazynin", L"", 0},
                            {L" Ruslan <IronFist> Tchernyi", L"", 0},
                            {L" Nina <Nina> Vatulich", L"", 0},

                            {L"Game balancing...", L"", 7000},
                            {L" Alexander <Alexartist> Yazynin", L"", 0},
                            {L"", L"Maps", 0},
                            {L" Dmitry <Dm> Gusarov", L"", 0},
                            {L"", L"Items", 0},
                            {L" Nina <Nina> Vatulich", L"", 0},
                            {L"", L"Maps", 0},

                            {L"Game texts and sounds...", L"", 5000},
                            {L" Ilia <Ilik> Plusnin", L"", 0},

                            {L"Thats all folks :)", L"", 3000},
                            {NULL, NULL, 0}};

void processCheat_DEVCON()
{
    g_MatrixMap->m_Console.SetActive(true);
}

void processCheat_SHOWFPS()
{
    g_MatrixMap->m_Console.SetActive(true);
}

void processCheat_INFO()
{
    INVERTFLAG(g_Config.m_DIFlags,
                   DI_TMEM | DI_TARGETCOORD | DI_VISOBJ | DI_ACTIVESOUNDS | DI_FRUSTUMCENTER);
}

void processCheat_AUTO()
{
    INVERTFLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE);
}

void processCheat_FLYCAM()
{
    INVERTFLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM);
}

void processCheat_SPAWN()
{
    D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;

    SSpecialBot bot{};

    bot.m_Chassis.m_nKind = RUK_CHASSIS_ANTIGRAVITY;
    bot.m_Armor.m_Unit.m_nKind = RUK_ARMOR_FULLSTACK;
    bot.m_Head.m_nKind = RUK_HEAD_DYNAMO;

    bot.m_Weapon[0].m_Unit.m_nKind = RUK_WEAPON_LASER;
    bot.m_Weapon[1].m_Unit.m_nKind = RUK_WEAPON_LASER;
    bot.m_Weapon[2].m_Unit.m_nKind = RUK_WEAPON_LASER;
    bot.m_Weapon[3].m_Unit.m_nKind = RUK_WEAPON_LASER;

    CMatrixRobotAI *r = bot.GetRobot(pos, PLAYER_SIDE);

    g_MatrixMap->AddObject(r, true);

    r->JoinToGroup();
    r->CreateTextures();
    r->InitMaxHitpoint(10000.0);

    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
    g_MatrixMap->GetPlayerSide()->PGOrderStop(g_MatrixMap->GetPlayerSide()->RobotToLogicGroup(r));
    RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
}

void processCheat_BABKI()
{
    INVERTFLAG(g_Config.m_DIFlags, DI_SIDEINFO);
}

void processCheat_CRAZYBOT()
{
    g_MatrixMap->GetPlayerSide()->BuildCrazyBot();
}

void processCheat_HURRY()
{
    if (!g_MatrixMap->MaintenanceDisabled() && g_MatrixMap->BeforeMaintenanceTime() > 0)
    {
        g_MatrixMap->SetMaintenanceTime(1);
    }
}

void processCheat_MEGABUST()
{
    // CSound::Play(S_BENTER, SL_INTERFACE);
    // CSound::Play(S_MAINTENANCE_ON, SL_INTERFACE);
    // CSound::Play(S_ROBOT_BUILD_END, SL_ALL);
    // CSound::Play(S_TERRON_KILLED, SL_ALL);
    // CSound::Play(S_TURRET_BUILD_2, SL_ALL);

    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_MEGABUSTALREADY))
    // if (0)
    {
        CMatrixMapStatic *s = CMatrixMapStatic::GetFirstLogic();
        for (; s; s = s->GetNextLogic()) {
            if (s->GetSide() == PLAYER_SIDE) {
                if (s->IsRobot() && !s->AsRobot()->IsAutomaticMode()) {
                    s->AsRobot()->MustDie();
                }
                // else if (s->IsCannon())
                //{
                //    s->AsCannon()->InitMaxHitpoint(s->AsCannon()->GetMaxHitPoint()*20);
                //} else if (s->IsBuilding())
                //{
                //    s->AsBuilding()->InitMaxHitpoint(s->AsBuilding()->GetMaxHitPoint()*20);
                //}
            }
        }
    }
    else {
        CMatrixMapStatic *s = CMatrixMapStatic::GetFirstLogic();
        for (; s; s = s->GetNextLogic()) {
            if (s->GetSide() == PLAYER_SIDE) {
                if (s->IsRobot()) {
                    s->AsRobot()->InitMaxHitpoint(s->AsRobot()->GetMaxHitPoint() *
                                                  20);
                }
                else if (s->IsCannon()) {
                    s->AsCannon()->InitMaxHitpoint(s->AsCannon()->GetMaxHitPoint() *
                                                   20);
                }
                else if (s->IsBuilding()) {
                    s->AsBuilding()->InitMaxHitpoint(
                            s->AsBuilding()->GetMaxHitPoint() * 20);
                }
            }
        }
        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_MEGABUSTALREADY);
    }
}

void processCheat_STATS()
{
    if (!g_MatrixMap->IsPaused())
    {
        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG_D);
    }
}

void processCheat_VIDEO()
{
    g_MatrixMap->m_DI.T(L"_____________________________", L"", 10000);
    g_MatrixMap->m_DI.T(L"Sim textures", utils::format(L"%u", g_D3DDCaps.MaxSimultaneousTextures).c_str(), 10000);
    g_MatrixMap->m_DI.T(L"Stencil available", FLAG(g_Flags, GFLAG_STENCILAVAILABLE) ? L"Yes" : L"No", 10000);

    // vidmode
    D3DDISPLAYMODE d3ddm;
    for (int i = 0; i < 2; ++i)
    {
        std::wstring modet = utils::format(L"Buffer %d mode", i);
        std::wstring modev;
        ASSERT_DX(g_D3DD->GetDisplayMode(0, &d3ddm));
        if (d3ddm.Format == D3DFMT_X8R8G8B8) {
            modev = L"X8R8G8B8";
        }
        else if (d3ddm.Format == D3DFMT_A8R8G8B8) {
            modev = L"A8R8G8B8";
        }
        else if (d3ddm.Format == D3DFMT_R8G8B8) {
            modev = L"R8G8B8";
        }
        else if (d3ddm.Format == D3DFMT_R5G6B5) {
            modev = L"R5G6B5";
        }
        else {
            modev = utils::format(L"%d", d3ddm.Format);
        }
        g_MatrixMap->m_DI.T(modet.c_str(), modev.c_str(), 10000);
    }
}

void processCheat_IAMLOOSER()
{
    g_ExitState = 3;
    g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_WIN);
}

void processCheat_BUBUBU()
{
    int delay = 0;
    int ctime = 0;
    int od = 0;
    for (int i = 0; stuff[i].t1 != NULL; ++i)
    {
        if (stuff[i].time)
        {
            od = delay;
            ctime = stuff[i].time;
            delay += stuff[i].time + 100;
        }
        g_MatrixMap->m_DI.T(stuff[i].t1, stuff[i].t2, ctime, od, true);
    }
}

void processCheat_RICHIERICH()
{
    CMatrixSideUnit* s = g_MatrixMap->GetPlayerSide();
    if (s)
    {
       s->AddResourceAmount(TITAN, 9000);
       s->AddResourceAmount(ELECTRONICS, 9000);
       s->AddResourceAmount(ENERGY, 9000);
       s->AddResourceAmount(PLASMA, 9000);
   }
}

void processCheat_KEEPALIVE()
{
    INVERTFLAG(g_Flags, GFLAG_KEEPALIVE);
    g_MatrixMap->m_DI.T(L"KEEPALIVE flag is set to ", FLAG(g_Flags, GFLAG_KEEPALIVE) ? L"true" : L"false", 1000);
}

void processCheat_NEED4SPEED()
{
    INVERTFLAG(g_Flags, GFLAG_4SPEED);
    g_MatrixMap->m_DI.T(L"4SPEED flag is set to ", FLAG(g_Flags, GFLAG_4SPEED) ? L"true" : L"false", 1000);
}

void processCheat_IAMTESTER()
{
    SETFLAG(g_Config.m_DIFlags, DI_SIDEINFO);
    SETFLAG(g_Config.m_DIFlags, DI_DRAWFPS);
    SETFLAG(g_Flags, GFLAG_KEEPALIVE);
    SETFLAG(g_Flags, GFLAG_4SPEED);
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE);
}

void processCheat_CRASH()
{
    abort();
}

bool IsInputEqual(const std::deque<uint8_t>& input, std::string_view str)
{
    if (input.size() < str.size())
    {
        return false;
    }

    auto istr = str.rbegin();
    auto iscan = input.rbegin();
    for (;
         istr != str.rend();
         istr++, iscan++)
    {
        if (*istr != VKey2Char(*iscan))
        {
            return false;
        }
    }

    return true;
}

bool Cheats::processInput(uint8_t vk)
{
    using cheatProc = std::pair<std::string_view, std::function<void()>>;

    const static std::vector<cheatProc> cheats{
        {"DEVCON",     processCheat_DEVCON},
        {"~",          processCheat_DEVCON},
        {"SHOWFPS",    processCheat_SHOWFPS},
        {"INFO ",      processCheat_INFO}, // note space at the end
        {"AUTO",       processCheat_AUTO},
        {"FLYCAM",     processCheat_FLYCAM},
        {"SPAWN",      processCheat_SPAWN}, // spawn a robot at cursor
        {"BABKI",      processCheat_BABKI},
        {"CRAZYBOT",   processCheat_CRAZYBOT},
        {"HURRY",      processCheat_HURRY},
        {"MEGABUST",   processCheat_MEGABUST},
        {"STATS",      processCheat_STATS},
        {"VIDEO",      processCheat_VIDEO},
        {"IAMLOOSER",  processCheat_IAMLOOSER},
        {"BUBUBU ",    processCheat_BUBUBU}, // note space at the end
        {"RICHIERICH", processCheat_RICHIERICH},
        {"KEEPALIVE",  processCheat_KEEPALIVE},
        {"NEED4SPEED", processCheat_NEED4SPEED},
        {"IAMTESTER",  processCheat_IAMTESTER},
        {"CRASH",      processCheat_CRASH},
    };

    constexpr int MAX_SCANS = 16;
    static std::deque<uint8_t> input{MAX_SCANS};
    if (input.size() == MAX_SCANS)
    {
        input.pop_front();
    }
    input.push_back(vk);

    auto iter =
        std::ranges::find_if(
            cheats,
            [](const auto& item){ return IsInputEqual(input, item.first); });

    if (iter != cheats.end())
    {
        input.clear();
        iter->second();
        return true;
    }

    return false;
}