#include "types.h"

std::vector<sead::SafeStringBase<char>> sceneName = {
"Shop",     
"Customize",     
"Plaza",     
"Tutorial",     
"PlayerMake",     
"MissionOcta",     
"BaseCampOcta",     
"DepartureOcta",     
"CentralOcta",     
"ShootingRange",     
"StaffRoll",     
"StaffRollOcta",    
"Lobby/Versus",     
"Lobby/Coop",     
"Lobby/Local",     
"Lobby/Lan",     
"Match/Versus",     
"Match/Coop",     
"Match/ForShow",     
"DayChange",     
"TitleForShow",     
"ThanksForShow",     
"LobbyForShow",     
"MiniGame",     
"DbgSetting"
};

for (auto& scene : sceneName) {    
  m_TextWriter->printf("%s", scene);
}