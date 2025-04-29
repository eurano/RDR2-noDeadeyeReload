#include "script.h"
#include "natives.h"
#include "types.h"
#include "nativeCaller.h"
#include "enums.h"
#include <string>


// ---------------------------------------------
// Struct definition for GUID
struct sGuid {
    alignas(8) int data1;
    alignas(8) int data2;
    alignas(8) int data3;
    alignas(8) int data4;
};

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

// ---------------------------------------------
// Main update function (call every tick)
void update() {
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    bool deadEyeActive = PLAYER::_0xB16223CB7DA965F0(player);
    static bool deadEyeWasActive = false;

    bool isReloading = PED::IS_PED_RELOADING(playerPed);

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

    // Debug info
    char buffer[256];
    sprintf_s(buffer, "DeadEye: %d | Tracking: %d | Ammo L: %d / R: %d", deadEyeActive, trackingAmmo, currentAmmoLeft, currentAmmoRight);
    draw_text(buffer, 960, 100);

    // Begin tracking when DeadEye activates
    if (deadEyeActive && !trackingAmmo) {
        trackingAmmo = true;
    }

    // If ammo increased during DeadEye (auto refill), restore saved values
    if (trackingAmmo && deadEyeActive && !isReloading && previousAmmoLeft >= 0 && previousAmmoRight >= 0) {
        if (currentAmmoRight > previousAmmoRight) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidRight, previousAmmoRight);
            draw_text("Restored RIGHT clip", 960, 150);
        }

        if (currentAmmoLeft > previousAmmoLeft) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidLeft, previousAmmoLeft);
            draw_text("Restored LEFT clip", 960, 180);
        }
    }

    // Reset tracking when DeadEye ends
    if (!deadEyeActive && trackingAmmo) {
        trackingAmmo = false;
        draw_text("DeadEye ended — tracking OFF", 960, 220);
    }

    if (!isReloading) {
        previousAmmoRight = currentAmmoRight;
        previousAmmoLeft = currentAmmoLeft;;
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


