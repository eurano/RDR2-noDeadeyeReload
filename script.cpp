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
bool gotRight = false;
bool gotLeft = false;

void update() {
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    bool deadEyeActive = PLAYER::_0xB16223CB7DA965F0(player);
    bool isReloading = PED::IS_PED_RELOADING(playerPed);

    int previousAmmoInClip = storedPreviousAmmoInClip;

    Hash currentWeapon = 0;
    int currentAmmoInClip = 0;

    bool hasWeapon = WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, true, 0, false);

    bool isPreviousBow = WEAPON::_0xC4DEC3CA8C365A5D(previousWeaponHash);
	bool isPreviousThrowable = WEAPON::_0x30E7C16B12DA8211(previousWeaponHash);

    if (hasWeapon) {
        WEAPON::GET_AMMO_IN_CLIP(playerPed, &currentAmmoInClip, currentWeapon);
        isCurrentTwoHanded = WEAPON::_0x0556E9D2ECF39D01(currentWeapon);
    }
    else {
        isCurrentTwoHanded = false;
    }

    sGuid guidRight = {};
    sGuid guidLeft = {};

    if (hasWeapon) {
        gotRight = WEAPON::_0x6929E22158E52265(playerPed, 0, (Any*)&guidRight);
        gotLeft = WEAPON::_0x6929E22158E52265(playerPed, 1, (Any*)&guidLeft);
    }

    int currentAmmoRight = 0;
    int currentAmmoLeft = 0;

    bool gotAmmoRight = false;
    bool gotAmmoLeft = false;

    if (gotRight) {
        gotAmmoRight = WEAPON::_0x678F00858980F516(playerPed, (Any*)&currentAmmoRight, (Any*)&guidRight);
    }

    if (gotLeft) {
        gotAmmoLeft = WEAPON::_0x678F00858980F516(playerPed, (Any*)&currentAmmoLeft, (Any*)&guidLeft);
    }


    if (deadEyeActive && !trackingAmmo && previousWeaponHash != -1569615261) {
        trackingAmmo = true;
    }

    if (trackingAmmo && hasWeapon && isCurrentTwoHanded && currentAmmoInClip > previousAmmoInClip && reloadedTwoHandedOnce == false && previousAmmoInClip != -1 && !isReloading && !isPreviousBow && !isPreviousThrowable) {
        WEAPON::SET_AMMO_IN_CLIP(playerPed, currentWeapon, previousAmmoInClip);
        reloadedTwoHandedOnce = true;
    }

    if (trackingAmmo && deadEyeActive && hasWeapon && !isReloading && previousAmmoLeft >= 0 && previousAmmoRight >= 0 && !isCurrentTwoHanded && !reloadedTwoHandedOnce && !isPreviousBow && !isPreviousThrowable) {

        if (gotRight && gotAmmoRight && currentAmmoRight > previousAmmoRight && previousAmmoRight != -1 && !reloadedRightOnce) {
            WEAPON::_0xDF4A3404D022ADDE(playerPed, (Any*)&guidRight, previousAmmoRight);
            reloadedRightOnce = true;
        }

        if (gotLeft && gotAmmoLeft && currentAmmoLeft > previousAmmoLeft && previousAmmoLeft != -1 && !isReloading && !reloadedLefttOnce) {
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
