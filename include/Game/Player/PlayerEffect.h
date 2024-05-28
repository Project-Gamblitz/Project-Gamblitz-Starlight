#pragma once

#include "types.h"
#include "Cmn/EffectManualHandle.h"

namespace Game {
	class Player;
	class PlayerEffect{
		public:
		void emitAndPlay_SuperArmorSt();
		void emitAndPlay_SuperArmor();
		void emitAndPlay_StartMarked(const Game::Player *);
		void emitAndPlay_Found();
		void emitAndPlay_Recover();
		void emitAndPlay_Vanish();
		void emitAndPlay_BarrierOn();
		void emitAndPlay_GetArmor();
		void emitAndPlay_LostArmor();
		void emitAndPlay_Special();
		void emitAndPlay_FailMarked();
		void emitAndPlay_StartDeviled();
		void emitAndPlay_Poisoned();
		void emitAndPlay_SuperArmorUse();
		void emitAndPlay_SuperArmorEnergyAbsorb(bool);
		void emitAndPlay_SuperArmorEd();
		void emitAndPlay_SuperArmorVanish();
		void emitAndPlay_SuperArmorBreak();
		void emitAndPlay_JetpackWarning();
		void emitAndPlay_SuperLandingCharge();
		void calcDraw(bool);

		// emitAndPlay_FailMarked
		Game::Player *mPlayer;
		_BYTE _8[0xD08];
		Cmn::EffectManualHandle mEffectManualHandle;
	};
};