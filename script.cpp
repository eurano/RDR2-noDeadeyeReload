#include "script.h"
#include <string>
#include <vector>

#if _MSC_VER > 1800  
struct sGuid {
    alignas(8) int data1;
    alignas(8) int data2;
    alignas(8) int data3;
    alignas(8) int data4;
};
#else
struct sGuid {
    __declspec(align(8)) int data1;
    __declspec(align(8)) int data2;
    __declspec(align(8)) int data3;
    __declspec(align(8)) int data4;
};
#endif

static bool trackingAmmo = false;
static int previousAmmoRight = -1;
static int previousAmmoLeft = -1;
static bool deadEyeWasActive = false;
static bool reloadedRightOnce = false;
static bool reloadedLefttOnce = false;
static bool reloadedTwoHandedOnce = false;
static Hash previousWeaponHash = 0;
static bool isCurrentTwoHanded = false;
static int storedPreviousAmmoInClip = -1;

void update() {
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    bool deadEyeActive = PLAYER::_0xB16223CB7DA965F0(player);
    bool isReloading = PED::IS_PED_RELOADING(playerPed);

    int previousAmmoInClip = storedPreviousAmmoInClip;

    sGuid guidRight = {};
    sGuid guidLeft = {};
    bool gotRight = WEAPON::_0x6929E22158E52265(playerPed, 0, (Any*)&guidRight);
    bool gotLeft = WEAPON::_0x6929E22158E52265(playerPed, 1, (Any*)&guidLeft);

    int currentAmmoRight = 0;
    int currentAmmoLeft = 0;
    bool gotAmmoRight = WEAPON::_0x678F00858980F516(playerPed, (Any*)&currentAmmoRight, (Any*)&guidRight);
    bool gotAmmoLeft = WEAPON::_0x678F00858980F516(playerPed, (Any*)&currentAmmoLeft, (Any*)&guidLeft);

    Hash currentWeapon = 0;
    int currentAmmoInClip = 0;

    if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, true, 0, false)) {
        WEAPON::GET_AMMO_IN_CLIP(playerPed, &currentAmmoInClip, currentWeapon);
    }

    isCurrentTwoHanded = WEAPON::_0x0556E9D2ECF39D01(currentWeapon);

    if (deadEyeActive && !trackingAmmo && previousWeaponHash != -1569615261) {
        trackingAmmo = true;
    }


    if (trackingAmmo && isCurrentTwoHanded && currentAmmoInClip > previousAmmoInClip && reloadedTwoHandedOnce == false && previousAmmoInClip != -1 && !isReloading) {
        WEAPON::SET_AMMO_IN_CLIP(playerPed, currentWeapon, previousAmmoInClip);
        reloadedTwoHandedOnce = true;
      }

    if (trackingAmmo && deadEyeActive && !isReloading && previousAmmoLeft >= 0 && previousAmmoRight >= 0 && !isCurrentTwoHanded && !reloadedTwoHandedOnce) {

        if (currentAmmoRight > previousAmmoRight && previousAmmoRight != -1 && !reloadedRightOnce) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidRight, previousAmmoRight);
            reloadedRightOnce = true;
        }

        if (currentAmmoLeft > previousAmmoLeft && previousAmmoLeft != -1 && !isReloading && !reloadedLefttOnce) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidLeft, previousAmmoLeft);
            reloadedLefttOnce = true;
        }
    }

    if (!deadEyeActive && trackingAmmo) {
        trackingAmmo = false;
        reloadedRightOnce = false;
        reloadedLefttOnce = false;
        reloadedTwoHandedOnce = false;
    }

    if (!isReloading && !deadEyeActive) {
        previousAmmoRight = currentAmmoRight;
        previousAmmoLeft = currentAmmoLeft;
        storedPreviousAmmoInClip = currentAmmoInClip;
        previousWeaponHash = currentWeapon;
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
