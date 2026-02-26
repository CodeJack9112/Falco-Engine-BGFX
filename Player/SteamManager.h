#pragma once

#ifdef _WIN32
namespace GX
{
	class SteamManager
	{
	private:
		STEAM_CALLBACK(SteamManager, OnGameOverlayActivated, GameOverlayActivated_t);
		STEAM_CALLBACK(SteamManager, OnAvatarLoaded, AvatarImageLoaded_t);
		STEAM_CALLBACK(SteamManager, UserStatsReceived, UserStatsReceived_t);
		STEAM_CALLBACK(SteamManager, UserStatsStored, UserStatsStored_t);
		STEAM_CALLBACK(SteamManager, UserAchievementStored, UserAchievementStored_t);
		STEAM_CALLBACK(SteamManager, UserAchievementIconFetched, UserAchievementIconFetched_t);

		MonoObject* getSteamID(CSteamID id);
	};

	MonoObject* SteamManager::getSteamID(CSteamID id)
	{
		MonoClass* klass2 = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/User/SteamID");

		MonoClassField* accountID = mono_class_get_field_from_name(klass2, "accountID");
		MonoClassField* accountInstance = mono_class_get_field_from_name(klass2, "accountInstance");
		MonoClassField* accountType = mono_class_get_field_from_name(klass2, "accountType");
		MonoClassField* universe = mono_class_get_field_from_name(klass2, "universe");
		MonoClassField* steamID = mono_class_get_field_from_name(klass2, "steamID");

		MonoObject* steamIDObj = mono_object_new(APIManager::getSingleton()->getDomain(), klass2);

		AccountID_t _id = id.GetAccountID();
		uint32 _inst = id.GetUnAccountInstance();
		EAccountType _type = id.GetEAccountType();
		EUniverse _uni = id.GetEUniverse();
		uint64 _id64 = id.ConvertToUint64();

		mono_field_set_value(steamIDObj, accountID, &_id);
		mono_field_set_value(steamIDObj, accountInstance, &_inst);
		mono_field_set_value(steamIDObj, accountType, &_type);
		mono_field_set_value(steamIDObj, universe, &_uni);
		mono_field_set_value(steamIDObj, steamID, &_id64);

		return steamIDObj;
	}

	void SteamManager::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
	{
		MonoClass* klass = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/Friends");

		bool active = pCallback->m_bActive == 1 ? true : false;
		void* params[1] = { &active };

		APIManager::getSingleton()->executeStatic(klass, "SteamAPI.Friends:RunGameOverlayActivatedCallbacks(bool)", params);
	}

	void SteamManager::OnAvatarLoaded(AvatarImageLoaded_t* pCallback)
	{
		CSteamID id = pCallback->m_steamID;

		MonoClass* klass = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/Friends");

		MonoObject* steamIDObj = getSteamID(id);
		void* _steamIDObj = mono_object_unbox(steamIDObj);

		void* params[4] = { _steamIDObj, &pCallback->m_iImage, &pCallback->m_iWide, &pCallback->m_iTall };

		APIManager::getSingleton()->executeStatic(klass, "SteamAPI.Friends:RunAvatarImageLoadedCallbacks(SteamAPI/User/SteamID,int,int,int)", params);
	}

	void SteamManager::UserStatsReceived(UserStatsReceived_t* pCallback)
	{
		MonoClass* klass = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/UserStats");

		MonoObject* steamIDObj = getSteamID(pCallback->m_steamIDUser);
		void* _steamIDObj = mono_object_unbox(steamIDObj);

		int result = static_cast<int>(pCallback->m_eResult);
		uint64 id = pCallback->m_nGameID;

		void* params[3] = { &id, &result, _steamIDObj };

		APIManager::getSingleton()->executeStatic(klass, "SteamAPI.UserStats:RunUserStatsReceivedCallbacks(long,int,SteamAPI/User/SteamID)", params);
	}

	void SteamManager::UserStatsStored(UserStatsStored_t* pCallback)
	{
		MonoClass* klass = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/UserStats");

		int result = static_cast<int>(pCallback->m_eResult);
		uint64 id = pCallback->m_nGameID;

		void* params[2] = { &id, &result };

		APIManager::getSingleton()->executeStatic(klass, "SteamAPI.UserStats:RunUserStatsStoredCallbacks(long,int)", params);
	}

	void SteamManager::UserAchievementStored(UserAchievementStored_t* pCallback)
	{
		MonoClass* klass = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/UserStats");

		std::string name = pCallback->m_rgchAchievementName;
		MonoString* nameStr = mono_string_new(APIManager::getSingleton()->getDomain(), name.c_str());
		uint64 id = pCallback->m_nGameID;
		int curProgress = (int)pCallback->m_nCurProgress;
		int maxProgress = (int)pCallback->m_nMaxProgress;

		void* params[4] = { &id, nameStr, &curProgress, &maxProgress };

		APIManager::getSingleton()->executeStatic(klass, "SteamAPI.UserStats:RunUserAchievementStoredCallbacks(long,string,int,int)", params);
	}

	void SteamManager::UserAchievementIconFetched(UserAchievementIconFetched_t* pCallback)
	{
		MonoClass* klass = mono_class_from_name(APIManager::getSingleton()->getBaseImage(), "FalcoEngine", "SteamAPI/UserStats");

		std::string name = pCallback->m_rgchAchievementName;
		MonoString* nameStr = mono_string_new(APIManager::getSingleton()->getDomain(), name.c_str());
		uint64 id = pCallback->m_nGameID.ToUint64();
		int achieved = pCallback->m_bAchieved;
		int icon = pCallback->m_nIconHandle;

		void* params[4] = { &id, nameStr, &achieved, &icon };

		APIManager::getSingleton()->executeStatic(klass, "SteamAPI.UserStats:RunUserAchievementIconFetchedCallbacks(long,string,bool,int)", params);
	}
}
#endif