#include "script.h"
#include "natives.h"
#include "types.h"
#include "nativeCaller.h"
#include "enums.h"
#include "main.h"
#include <string>

#if _MSC_VER > 1800  
struct sGuid {
	alignas(8) int data1;
	alignas(8) int data2;
	alignas(8) int data3;
	alignas(8) int data4;
};
#else   //if Visual Studio 2013
struct sGuid {
	__declspec(align(8)) int data1;
	__declspec(align(8)) int data2;
	__declspec(align(8)) int data3;
	__declspec(align(8)) int data4;
};
#endif

// ---------------------------------------------
// Helper: Draw on-screen text for debugging
void draw_text(const char* str, float x, float y) {
    float fX = x / 1920.0f;
    float fY = y / 1080.0f;

    UI::SET_TEXT_SCALE(0.2f, 0.2f);
    UI::SET_TEXT_COLOR_RGBA(255, 255, 255, 255);
    UI::SET_TEXT_CENTRE(1);
    UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);

    char buffer[256];
    strcpy_s(buffer, sizeof(buffer), "LITERAL_STRING");

    UI::DRAW_TEXT(
        GAMEPLAY::CREATE_STRING(10, buffer, const_cast<char*>(str)),
        fX,
        fY
    );
}

// ---------------------------------------------
// Persistent tracking variables
static bool trackingAmmo = false;
static int previousAmmoRight = -1;
static int previousAmmoLeft = -1;
static bool deadEyeWasActive = false;
static bool reloadedRightOnce = false;
static bool reloadedLefttOnce = false;
static bool reloadedTwoHandedOnce = false;

static bool isCurrentTwoHanded = false;
static int storedPreviousAmmoInClip = -1;
static Hash previousWeapon = 0;

// ---------------------------------------------
// Main update function (call every tick)
void update() {
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    bool deadEyeActive = PLAYER::_0xB16223CB7DA965F0(player);
    static bool deadEyeWasActive = false;
    bool isReloading = PED::IS_PED_RELOADING(playerPed);

    // Cache previous ammo clip before it's overwritten
    int previousAmmoInClip = storedPreviousAmmoInClip;

    // GUIDs for right and left weapons
    sGuid guidRight = {};
    sGuid guidLeft = {};
    bool gotRight = WEAPON::_0x6929E22158E52265(playerPed, 0, (Any*)&guidRight);
    bool gotLeft = WEAPON::_0x6929E22158E52265(playerPed, 1, (Any*)&guidLeft);

    // Ammo values
    int currentAmmoRight = 0;
    int currentAmmoLeft = 0;
    bool gotAmmoRight = WEAPON::_0x678F00858980F516(playerPed, (Any*)&currentAmmoRight, (Any*)&guidRight);
    bool gotAmmoLeft = WEAPON::_0x678F00858980F516(playerPed, (Any*)&currentAmmoLeft, (Any*)&guidLeft);

    Hash currentWeapon = 0;
    int currentAmmoInClip = 0;

    if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, true, 0, false)) {
        WEAPON::GET_AMMO_IN_CLIP(playerPed, &currentAmmoInClip, currentWeapon); //there is error in this function in SDK natives.h wich has to be fixed manually
    }

    isCurrentTwoHanded = WEAPON::_0x0556E9D2ECF39D01(currentWeapon);

    // Debug info
    char buffera[256];
    sprintf_s(buffera, "DeadEye: %d | Tracking: %d | Ammo L: %d / R: %d", deadEyeActive, trackingAmmo, currentAmmoLeft, currentAmmoRight);
    draw_text(buffera, 960, 100);

    char bufferx[256];
    sprintf_s(bufferx, "isCurrentTwoHanded: %d", isCurrentTwoHanded);
    draw_text(bufferx, 960, 120);

    char buffery[256];
    sprintf_s(buffery, "previousAmmoInClip: %d", previousAmmoInClip);
    draw_text(buffery, 960, 140);

    char bufferv[256];
    sprintf_s(bufferv, "currentAmmoInClip: %d", currentAmmoInClip);
    draw_text(bufferv, 960, 160);

    char bufferh[256];
    sprintf_s(bufferh, "currentWeaponHASH: %d", currentWeapon);
    draw_text(bufferh, 960, 180);

    char bufferf[256];
    sprintf_s(bufferf, "reloadedTwoHandedOnce: %d", reloadedTwoHandedOnce);
    draw_text(bufferf, 960, 200);



    // Start tracking
    if (deadEyeActive && !trackingAmmo) {
        trackingAmmo = true;
    }

    // Two-handed weapon clip restore
    if (trackingAmmo && isCurrentTwoHanded && currentAmmoInClip > previousAmmoInClip && reloadedTwoHandedOnce == false && previousAmmoInClip > -1 && !isReloading) {
        WEAPON::SET_AMMO_IN_CLIP(playerPed, currentWeapon, previousAmmoInClip);
        reloadedTwoHandedOnce = true;
    }

    // Dual-wield weapon restore
    if (trackingAmmo && deadEyeActive && !isReloading && previousAmmoLeft >= 0 && previousAmmoRight >= 0 && !isCurrentTwoHanded && !reloadedTwoHandedOnce) {

        if (currentAmmoRight > previousAmmoRight && previousAmmoRight != -1 && !reloadedRightOnce) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidRight, previousAmmoRight);
            reloadedRightOnce = true;
        }

        if (currentAmmoLeft > previousAmmoLeft && !isReloading && !reloadedLefttOnce) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidLeft, previousAmmoLeft);
            reloadedLefttOnce = true;
            draw_text("Restored LEFT clip", 960, 220);
        }
    }

    // Reset tracking when DeadEye ends
    if (!deadEyeActive && trackingAmmo) {
        trackingAmmo = false;
        reloadedRightOnce = false;
        reloadedLefttOnce = false;
        reloadedTwoHandedOnce = false;
        draw_text("DeadEye ended — tracking OFF", 960, 240);
    }

    // Save ammo info when not reloading
    if (!isReloading && !deadEyeActive) {
        previousAmmoRight = currentAmmoRight;
        previousAmmoLeft = currentAmmoLeft;
        storedPreviousAmmoInClip = currentAmmoInClip;  // <- FIXED! Only update after comparisons
    }

    deadEyeWasActive = deadEyeActive;
}

void main() {
    while (true) {
        update();
        WAIT(0);
    }
}

void ScriptMain() {
    srand(GetTickCount());
    main();
}
