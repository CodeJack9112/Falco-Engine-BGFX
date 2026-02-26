#include "API_SteamAPI.h"

#include "../steam/steam_api.h"
#include "../steam/steamclientpublic.h"

#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Engine/Serialization/Settings/ProjectSettings.h"

//////////////////////
namespace GX
{
#ifdef _WIN32
	MonoString* API_SteamAPI::getPersonaName()
	{
		if (SteamFriends() == nullptr)
			return mono_string_new(APIManager::getSingleton()->getDomain(), "");

		const char* name = SteamFriends()->GetPersonaName();

		return mono_string_new(APIManager::getSingleton()->getDomain(), name);
	}

	int API_SteamAPI::getPersonaState()
	{
		if (SteamFriends() == nullptr)
			return 0;

		EPersonaState state = SteamFriends()->GetPersonaState();

		return static_cast<int>(state);
	}

	bool API_SteamAPI::requestUserInformation(API::Steam::SteamID* ref_id, bool requireNameOnly)
	{
		if (SteamFriends() == nullptr)
			return false;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_id->steamID);

		return SteamFriends()->RequestUserInformation(id, requireNameOnly);
	}

	void API_SteamAPI::getSteamID(API::Steam::SteamID* out_value)
	{
		if (SteamUser() == nullptr)
			return;

		CSteamID id = SteamUser()->GetSteamID();

		out_value->accountID = id.GetAccountID();
		out_value->accountInstance = id.GetUnAccountInstance();
		out_value->accountType = static_cast<API::Steam::AccountType>(static_cast<int>(id.GetEAccountType()));
		out_value->universe = static_cast<API::Steam::Universe>(static_cast<int>(id.GetEUniverse()));
		out_value->steamID = id.ConvertToUint64();
	}

	int API_SteamAPI::getSmallFriendAvatar(API::Steam::SteamID* ref_value)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_value->steamID);

		int imgId = SteamFriends()->GetSmallFriendAvatar(id);

		return imgId;
	}

	int API_SteamAPI::getMediumFriendAvatar(API::Steam::SteamID* ref_value)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_value->steamID);

		int imgId = SteamFriends()->GetMediumFriendAvatar(id);

		return imgId;
	}

	int API_SteamAPI::getLargeFriendAvatar(API::Steam::SteamID* ref_value)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_value->steamID);

		int imgId = SteamFriends()->GetLargeFriendAvatar(id);

		return imgId;
	}

	void API_SteamAPI::getImageSize(int id, API::Vector2* out_value)
	{
		if (SteamUtils() == nullptr)
			return;

		uint32 w = 0;
		uint32 h = 0;

		SteamUtils()->GetImageSize(id, &w, &h);

		out_value->x = w;
		out_value->y = h;
	}

	MonoArray* API_SteamAPI::getImageRGBA(int id)
	{
		if (SteamUtils() == nullptr)
			return nullptr;

		uint32 w = 0;
		uint32 h = 0;

		SteamUtils()->GetImageSize(id, &w, &h);

		const int uImageSizeInBytes = w * h * 4;
		uint8* pAvatarRGBA = new uint8[uImageSizeInBytes];
		bool success = SteamUtils()->GetImageRGBA(id, pAvatarRGBA, uImageSizeInBytes);

		if (success)
		{
			MonoClass* sbyte = mono_class_from_name(APIManager::getSingleton()->getSystemImage(), "System", "SByte");
			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), sbyte, uImageSizeInBytes);
			for (int i = 0; i < uImageSizeInBytes; ++i)
				mono_array_set(arr, uint8, i, pAvatarRGBA[i]);

			return arr;
		}

		return nullptr;
	}

	void API_SteamAPI::activateGameOverlay(int overlayType)
	{
		if (SteamFriends() == nullptr)
			return;

		std::string dialog = "friends";

		switch (overlayType)
		{
		case 0:
			dialog = "friends";
			break;
		case 1:
			dialog = "community";
			break;
		case 2:
			dialog = "players";
			break;
		case 3:
			dialog = "settings";
			break;
		case 4:
			dialog = "officialgamegroup";
			break;
		case 5:
			dialog = "stats";
			break;
		case 6:
			dialog = "achievements";
			break;
		default:
			dialog = "friends";
			break;
		}

		SteamFriends()->ActivateGameOverlay(dialog.c_str());
	}

	void API_SteamAPI::activateGameOverlayToUser(int overlayType, API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return;

		std::string dialog = "steamid";

		switch (overlayType)
		{
		case 0:
			dialog = "steamid";
			break;
		case 1:
			dialog = "chat";
			break;
		case 2:
			dialog = "jointrade";
			break;
		case 3:
			dialog = "stats";
			break;
		case 4:
			dialog = "achievements";
			break;
		case 5:
			dialog = "friendadd";
			break;
		case 6:
			dialog = "friendremove";
			break;
		case 7:
			dialog = "friendrequestaccept";
			break;
		case 8:
			dialog = "friendrequestignore";
			break;
		default:
			dialog = "steamid";
			break;
		}

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		SteamFriends()->ActivateGameOverlayToUser(dialog.c_str(), id);
	}

	void API_SteamAPI::activateGameOverlayToWebPage(MonoString* url, int mode)
	{
		if (SteamFriends() == nullptr)
			return;

		std::string _url = mono_string_to_utf8(url);

		SteamFriends()->ActivateGameOverlayToWebPage(_url.c_str(), static_cast<EActivateGameOverlayToWebPageMode>(mode));
	}

	void API_SteamAPI::clearRichPresence()
	{
		if (SteamFriends() == nullptr)
			return;

		SteamFriends()->ClearRichPresence();
	}

	void API_SteamAPI::closeClanChatWindowInSteam(API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		SteamFriends()->CloseClanChatWindowInSteam(id);
	}

	int API_SteamAPI::getFriendCount(int flags)
	{
		if (SteamFriends() == nullptr)
			return 0;

		return SteamFriends()->GetFriendCount(flags);
	}

	void API_SteamAPI::getFriendByIndex(int flags, int index, API::Steam::SteamID* out_steamID)
	{
		if (SteamFriends() == nullptr)
			return;

		CSteamID id = SteamFriends()->GetFriendByIndex(index, flags);

		out_steamID->accountID = id.GetAccountID();
		out_steamID->accountInstance = id.GetUnAccountInstance();
		out_steamID->accountType = static_cast<API::Steam::AccountType>(static_cast<int>(id.GetEAccountType()));
		out_steamID->universe = static_cast<API::Steam::Universe>(static_cast<int>(id.GetEUniverse()));
		out_steamID->steamID = id.ConvertToUint64();
	}

	MonoString* API_SteamAPI::getFriendPersonaName(API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return mono_string_new(APIManager::getSingleton()->getDomain(), "");

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		const char* name = SteamFriends()->GetFriendPersonaName(id);

		return mono_string_new(APIManager::getSingleton()->getDomain(), name);
	}

	int API_SteamAPI::getFriendPersonaState(API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		return static_cast<int>(SteamFriends()->GetFriendPersonaState(id));
	}

	uint32 API_SteamAPI::getFriendCoplayGame(API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		return SteamFriends()->GetFriendCoplayGame(id);
	}

	int API_SteamAPI::getFriendRelationship(API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);
		
		EFriendRelationship val = SteamFriends()->GetFriendRelationship(id);

		return static_cast<int>(val);
	}

	int API_SteamAPI::getFriendRichPresenceKeyCount(API::Steam::SteamID* ref_steamID)
	{
		if (SteamFriends() == nullptr)
			return 0;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		return SteamFriends()->GetFriendRichPresenceKeyCount(id);
	}

	MonoString* API_SteamAPI::getFriendRichPresenceKeyByIndex(API::Steam::SteamID* ref_steamID, int index)
	{
		if (SteamFriends() == nullptr)
			return mono_string_new(APIManager::getSingleton()->getDomain(), "");

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		const char* key = SteamFriends()->GetFriendRichPresenceKeyByIndex(id, index);
		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), key);

		return str;
	}

	MonoString* API_SteamAPI::getFriendRichPresence(API::Steam::SteamID* ref_steamID, MonoString* key)
	{
		if (SteamFriends() == nullptr)
			return mono_string_new(APIManager::getSingleton()->getDomain(), "");

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		const char* strKey = mono_string_to_utf8(key);
		const char* val = SteamFriends()->GetFriendRichPresence(id, strKey);
		
		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), val);

		return str;
	}

	int API_SteamAPI::getNumAchievements()
	{
		if (SteamUserStats() == nullptr)
			return 0;

		return SteamUserStats()->GetNumAchievements();
	}

	MonoString* API_SteamAPI::getAchievementName(int index)
	{
		if (SteamUserStats() == nullptr)
			return nullptr;

		const char* name = SteamUserStats()->GetAchievementName(index);
		MonoString* strName = mono_string_new(APIManager::getSingleton()->getDomain(), name);

		return strName;
	}

	bool API_SteamAPI::requestCurrentStats()
	{
		if (SteamUserStats() == nullptr)
			return false;

		return SteamUserStats()->RequestCurrentStats();
	}

	bool API_SteamAPI::requestUserStats(API::Steam::SteamID* ref_steamID)
	{
		if (SteamUserStats() == nullptr)
			return false;

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		return SteamUserStats()->RequestUserStats(id);
	}

	bool API_SteamAPI::storeStats()
	{
		if (SteamUserStats() == nullptr)
			return false;

		return SteamUserStats()->StoreStats();
	}

	bool API_SteamAPI::getUserAchievement(API::Steam::SteamID* ref_steamID, MonoString* name)
	{
		if (SteamUserStats() == nullptr)
			return false;

		const char* nameStr = mono_string_to_utf8(name);

		CSteamID id = CSteamID();
		id.SetFromUint64(ref_steamID->steamID);

		bool value = false;
		SteamUserStats()->GetUserAchievement(id, nameStr, &value);

		return value;
	}

	bool API_SteamAPI::getAchievement(MonoString* name)
	{
		if (SteamUserStats() == nullptr)
			return false;

		const char* nameStr = mono_string_to_utf8(name);

		bool value = false;
		SteamUserStats()->GetAchievement(nameStr, &value);

		return value;
	}

	int API_SteamAPI::getAchievementIcon(MonoString* name)
	{
		if (SteamUserStats() == nullptr)
			return 0;

		const char* nameStr = mono_string_to_utf8(name);

		return SteamUserStats()->GetAchievementIcon(nameStr);
	}

	bool API_SteamAPI::setAchievement(MonoString* name)
	{
		if (SteamUserStats() == nullptr)
			return false;

		const char* nameStr = mono_string_to_utf8(name);

		return SteamUserStats()->SetAchievement(nameStr);
	}

	bool API_SteamAPI::clearAchievement(MonoString* name)
	{
		if (SteamUserStats() == nullptr)
			return false;

		const char* nameStr = mono_string_to_utf8(name);

		return SteamUserStats()->ClearAchievement(nameStr);
	}

	MonoString* API_SteamAPI::getAchievementDisplayAttribute(MonoString* name, int displayAttribute)
	{
		if (SteamUserStats() == nullptr)
			return mono_string_new(APIManager::getSingleton()->getDomain(), "");

		const char* nameStr = mono_string_to_utf8(name);

		const char* attr = "";

		switch (displayAttribute)
		{
		case 0:
			attr = "name";
			break;
		case 1:
			attr = "desc";
			break;
		case 2:
			attr = "hidden";
			break;
		}

		const char* value = SteamUserStats()->GetAchievementDisplayAttribute(nameStr, attr);
		MonoString* valueStr = mono_string_new(APIManager::getSingleton()->getDomain(), value);

		return valueStr;
	}
	#endif
}