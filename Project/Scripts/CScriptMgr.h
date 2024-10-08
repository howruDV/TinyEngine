#pragma once

#include <vector>
#include <string>

enum SCRIPT_TYPE
{
	CAMERAMOVESCRIPT,
	KIRBYFSM,
	KIRBYMOVECONTROLLER,
	MODELEDITORCAMERAMOVESCRIPT,
	PLAYERMGR,
	KIRBYBULLETSCRIPT,
	BUTTONSCRIPT,
	UIMANAGERSCRIPT,
	BUTTONMANAGERSCRIPT,
	KIRBYVACUUMCOLLIDER,
	KIRBYCOPYABILITYSCRIPT,
	KIRBYCOPYOBJSCRIPT,
	KIRBYBODYCOLLIDER,
	UITEXMANAGERSCRIPT,
	UITEXSCRIPT,
	UIANIMMANAGERSCRIPT,
	LEVELCHANGEBUTTONSCRIPT,
	STAGECHANGEBUTTONSCRIPT,
	EXITBUTTON,
	UIHPSCRIPT,
	UIUNITMANAGERSCRIPT,
	ENGINETESTSCRIPT,
	BLADEKNIGHTSCRIPT,
	DETECTAREASCRIPT,
	BLADEKNIGHTSWORDSCRIPT,
	NODDYSCRIPT,
	HOTHEADSCRIPT,
	KABUSCRIPT,
	SIRKIBBLESCRIPT,
	GHOSTGORDOSCRIPT,
	GORDOLONGSCRIPT,
	NORMALENEMYSCRIPT,
	TACKLEENEMYSCRIPT,
	CUTTERPROJECTILESCRIPT,
	FIREPROJECTILESCRIPT,
	ATTACKAREASCRIPT,
	DEFORMOBJECTSCRIPT,
	KIRBYHATBLADE,
	KIRBYCUTTERBULLET,
	CANJUICE,
	MOMENTARYOBJSCRIPT,
	KIRBYWEAPONHITBOX,
	CAMERACONTROLLER,
	UIMOVEUPDOWNSCRIPT,
	UIANIMSCRIPT,
	UIFLOWSCRIPT,
	UISTARTSCENEFLOWSCRIPT,
	UIGLOWMOVESCRIPT,
	CHANGECAMTRIGGER,
	PROGRESSCAMTRIGGER,
	TWOTARGETCAMTRIGGER,
	ELFILISFSM,
	BOSSMGR,
	ELFILISARROWSETSCRIPT,
	ELFILISARROWSCRIPT,
	ELFILISSWORDSLASHSCRIPT,
	ELFILISSTORMSCRIPT,
	ELFILISAIRARROW,
	ELFILISDIMENSIONLASER,
	ELFILISLASER,
	ELFILISBIGFSM,
	CHANGEALPHASCRIPT,
	CRUMBLESCRIPT,
	SURPRISEBOARDSCRIPT,
	SURPRISEBOARDATTACKSCRIPT,
	KIRBYUNITSCRIPT,
	SOLARCHARGESCRIPT,
	SOLARONCESCRIPT,
	ELEVATORSCRIPT,
	PUSHOUTSCRIPT,
	OBSTACLETRIGGERSCRIPT,
	LADDERSCRIPT,
	KIRBYCHECKPOINTTRIGGER,
	KIRBYFALLDETECTTRIGGER,
	FIXEDVIEWCAMTRIGGER,
	MORPHOFSM,
	MORPHOUNIT,
	ELFILISUNIT,
	MORPHOTRACKINGSOUL,
	MORPHOSHOCKWAVE,
	PUSHOUTCOLLIDERSCRIPT,
	LIGHTBULBGIMMICKSCRIPT,
	FADEEFFECTSCRIPT,
	LENSFLARESCRIPT,
	FLOWMGR_LV0,
	FLOWMGR_LV1,
	FLOWMGR_LV2,
	FLOWMGR_LV3,
	FLOWMGR_LV4,
	PHANTASCRIPT,
	SPOOKSTEPSCRIPT,
	SPAWNTRIGGERSCRIPT,
	FLOWMGR_BOSSELFILIS,
	FLOWTRIGGERSCRIPT,
	KIRBYLIGHTSCRIPT,
	BOSSHITBOX,
	FLOWMGR_BOSSMORPHO,
	FOLLOWPLAYERSCRIPT,
	STAGECLEAR,
	KIRBYSMOKE,
	UIROTATESCRIPT,
	UIMTRLSCRIPT,
	FLOWMGR_LVROBBY,
	FLOWMGR_LVSTART,
	UIFLOWMGR,
	KIRBYBULLETSIZE,
	KIRBYOBJDETECTCOLLIDER,
	SMOKESCRIPT,
	SMOKESPAWNER,
	UIBOSSHPSCRIPT,
	ENTERUISCRIPT,
	KIRBYDROPOUTUISCRIPT,
	PLAYERHITBOX,
	KIRBYCANTEAT,
	BOSSDROPSTARSCRIPT,
	KIRBYFIREBULLET,
	FIREONHITEFFECT,
	DAMAGESTAREFFECT,
	MONSTERDEADEFFECTSPAWNSCRIPT,
	SHOCKWAVESCRIPT,
	UIBURSTSTARSPAWNEFFECTSCRIPT,
	UICHANGEABILITYSTARSPAWNEFFECTSCRIPT,
	UICHANGEABILITYSTAREFFECT,
	UICHANGEABILITYSTOPSTAREFFECT,
	KIRBYSOUNDLISTENER,
	UIDASHEFFECTSCRIPT,
	DESTROYPARTICLESCRIPT,
	MORPHOSHOCKWAVEWAVE,
	KIRBYSWORDTORNADOSCRIPT,
	UICOMMONHITEFFECTSCRIPT,
	UISLASHEFFECTSCRIPT,
	UIABSORBUISCRIPT,
	UICONTINUEUISCRIPT,
	KIRBYVACUUMSCRIPT,
	AUTODELETE,
	END,
};

using namespace std;

class CScript;

class CScriptMgr
{
public:
	static void GetScriptInfo(vector<wstring>& _vec);
	static CScript* GetScript(const wstring& _strScriptName);
	static CScript* GetScript(UINT _iScriptType);
	static const wchar_t* GetScriptName(CScript* _pScript);
};
