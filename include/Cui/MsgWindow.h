#pragma once

#include "types.h"
#include "sead/string.h"

namespace nn { namespace ui2d {
	class TextBox {
	public:
		void SetStringUtf8(const char *str, unsigned short maxLen);
	};
}}

namespace Cui {

	// MsgArg: argument struct for MsgWindowPageHandler::in()
	// Size 0x330: main fields up to +0xE0, sub-object at +0xE0 extends to +0x328
	class MsgArg {
	public:
		MsgArg();
		void reset();
		void copy(MsgArg const &other);
		void readCommonMsgAttr(sead::SafeStringBase<char> const &label, sead::SafeStringBase<char> const &attr);

		_BYTE _0[0x8];
		// +0x8: window type byte (0=OK, 1=SingleBtn, 2=TwoBtn, 3=Plain, 4=Loading)
		u8 mWindowType;
		_BYTE _9[0x327];
	};

	class MsgWindowPage;

	// MsgWindowPageHandler: drives a single MsgWindowPage lifecycle
	class MsgWindowPageHandler {
	public:
		void in(MsgArg const &arg);
		void out();

		bool isAbleIn() const;
		bool isDecide() const;
		bool isEndDecide() const;
		int getDecideBtn() const; // 0=none, 1=OK, 2=single, 3=left, 4=right

		_BYTE _0[0x8];
		MsgWindowPage *mPage; // +0x8
		_BYTE _10[0x28];
	};

	// SystemPageMgr: singleton that owns MsgWindowPageHandlers
	class SystemPageMgr {
	public:
		static SystemPageMgr *sInstance;

		_BYTE _0[0xF8];
		// +0xF8: pointer to MsgWindowPageHandler for the main message window
		MsgWindowPageHandler *mMsgWindowHandler;
	};

}
