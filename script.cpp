#include "script.h"
#include "natives.h"
#include "types.h"
#include "nativeCaller.h"
#include "enums.h"

void update() {
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    static bool deadEyeWasActive = false;
    static int previousAmmoInClip = -1;
    static bool trackingAmmo = false;
    static Hash previousWeapon = 0; 
    bool deadEyeActive = PLAYER::_0xB16223CB7DA965F0(player);
    Hash currentWeapon = 0;
    int currentAmmoInClip = 0;

    if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, true, 0, false)) {
        if (WEAPON::GET_AMMO_IN_CLIP(playerPed, &currentAmmoInClip, currentWeapon)) {
            //
        }
    }

    if (deadEyeActive && !trackingAmmo) {
        trackingAmmo = true;
    }

    if (deadEyeActive && previousWeapon != currentWeapon) {
        previousAmmoInClip = -1;
        trackingAmmo = false;
        previousWeapon = currentWeapon;
    }

    if (trackingAmmo && deadEyeActive && previousAmmoInClip >= 0) {
        if (currentAmmoInClip > previousAmmoInClip) {
            WEAPON::SET_AMMO_IN_CLIP(playerPed, currentWeapon, previousAmmoInClip);
        }
    }

    if (!deadEyeActive && trackingAmmo) {
        trackingAmmo = false;
    }

    previousAmmoInClip = currentAmmoInClip;
    deadEyeWasActive = deadEyeActive;

    previousWeapon = currentWeapon;

    WAIT(0);
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
