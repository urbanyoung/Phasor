#pragma once

#include "../../Common/Streams.h"
#include <assert.h>

namespace halo { 
	struct s_player;

	extern const std::wstring MSG_PREFIX;

	// Writes to the server console.
	class CHaloPrintStream : public COutStream
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		CHaloPrintStream() {}
		virtual ~CHaloPrintStream() {}

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new CHaloPrintStream());
		}
	};

	class CPlayerBaseStream : public COutStream
	{
	private:
		// Memory id of player at last known point.
		// -1 unless stream has been cloned.
		int memory_id;
		// Hash of player at last known point
		std::string hash;
	protected:
		const s_player& player;

		CPlayerBaseStream(const s_player& player, bool do_check);
		CPlayerBaseStream(const s_player& player);

		// Checks if the player is still valid
		bool ValidatePlayer();

	public:
		const s_player& GetPlayer() { return player; }
	};

	// Writes console text to a specific player
	class PlayerConsoleStream : public CPlayerBaseStream
	{
	private:	
		// used when cloning. forces checks to be made before each Write.
		PlayerConsoleStream(const s_player& player, bool);

	protected:		
		virtual bool Write(const std::wstring& str) override;

	public:
		PlayerConsoleStream(const s_player& player);

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new PlayerConsoleStream(player,true));
		}
	};	

	class PlayerChatStreamRaw : public CPlayerBaseStream
	{
	protected:		
		virtual bool Write(const std::wstring& str) override;

		// used when cloning. forces checks to be made before each Write.
		PlayerChatStreamRaw(const s_player& player, bool);

	public:
		PlayerChatStreamRaw(const s_player& player);

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new PlayerChatStreamRaw(player,true));
		}
	};


	class PlayerChatStream : public PlayerChatStreamRaw
	{
	private:	
		// used when cloning. forces checks to be made before each Write.
		PlayerChatStream(const s_player& player, bool);

	protected:		
		virtual bool Write(const std::wstring& str) override;

	public:
		PlayerChatStream(const s_player& player);

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new PlayerChatStream(player,true));
		}
	};

}

