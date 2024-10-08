#pragma once

#include <Engine/CScript.h>

class CGameObject;
class CFadeEffectScript;
class CUIFlowScript;
class CEnterUIScript;

class CLevelFlowMgr : public CScript
{
private:
    bool m_bIsChangedLevel;

    bool m_bFadeEffect;
    float m_FadeEffectAcc;
    float m_FadeEffectDuration;

    bool m_bRadialBlurEffect;
    float m_RadialBlurAcc;
    float m_RadialBlurDuration;

    bool m_bDimensionalFade;
    float m_StartDimensionalCoef;
    float m_EndDimensionalCoef;
    float m_CurDimensionalCoef;
    float m_DimensionalAcc;
    float m_DimensionalDuration;

    wstring m_CurLevelPath;
    string m_NextLevelPath;

    // BGM
    Ptr<CSound> m_BGM;
    float m_StartBGMVolume;
    float m_EndBGMVolume;
    float m_BGMAcc;
    float m_BGMDuration;

    // UI
    CGameObject* m_DimensionFadeEffect;
    CGameObject* m_RadialBlurEffect;
    CGameObject* m_pLoadingUI;
    CGameObject* m_pPlayerHP;
    CGameObject* m_pBossHP;
    CGameObject* m_pDropUI;
    CEnterUIScript* m_pEnterUIScript;
    CGameObject* m_pClearUI;
    CGameObject* m_pContinueUI;
    CGameObject* m_pAbsorbUI;

    bool m_bEnterLevel;
    float m_fFadeInAccTime;
    float m_fLoadingAccTime;
    float m_fFadeInWaitTime;

    bool m_bStartLevel;
    bool m_bStartLevelDurationValue;
    bool m_bUILevel;
    bool m_bLoadingUIWait;
    bool m_bAlreayEnterBossUI;
    bool m_bContinuUIOn;

    CFadeEffectScript* m_FadeEffectScript;

    CUIFlowScript* m_UIFlowScript;
    Ptr<CMaterial> m_ToneMappingMtrl;

public:
    virtual void begin() override;
    virtual void tick() override;
    virtual void TriggerEvent(UINT _Idx){};

public:
    bool IsChangedLevel() const { return m_bIsChangedLevel; }

    void SetNextLevel(const string _string) { m_NextLevelPath = _string; }
    wstring& GetCurrentLevel() { return m_CurLevelPath; }

    void SetStartLevel(bool _bFlag) { m_bStartLevel = _bFlag; }
    void SetUILevel(bool _bFlag) { m_bUILevel = _bFlag; }

    void SetLoadingUIColor(Vec3 _Color);
    void SetEnterLevel(const bool _bFlag) { m_bEnterLevel = _bFlag; }

public:
    void OnDimensionFade(float _Coef);
    void OnDimensionFade(float _StartCoef, float _EndCoef, float _Duration);
    void OffDimensionFade();
    void OnRadialBlurEffect(float _Duration);
    void OffRadialBlurEffect();
    void SetRadialBlurEffect(float _Radius, float _BlurPower, Vec2 _UV);
    CGameObject* GetRadialBlurEffect() { return m_RadialBlurEffect; }
    void SetFadeEffectColor(Vec3 _Color);
    void SetToneMappingParam(bool _bBloomEnable = false, bool _bBlendMode = false, float _BloomStrength = 0.5f, float _Threshold = 0.f,
                             float _FilterRadius = 1.f, float _Exposure = 1.f, float _Gamma = 2.2f);

    void ActiveFadeEffect(bool _bEnable);
    void PauseFadeEffect(bool _bPause);
    void SetFadeEffect(Vec3 _Color, bool _bReverse, float _Duration, float _Speed, bool _CenterMode);

    void FadeInBGM(const wstring& _SoundPath, float _StartVolume, float _EndVolume, float _Duration);
    void FadeOutBGM(float _Duration);
    Ptr<CSound> GetBGM() const { return m_BGM; }
    float GetVolume() const { return m_EndBGMVolume; }

public:
    virtual void LevelStart();
    virtual void LevelEnd();
    virtual void LevelRestart();
    virtual void RobbyLevel();

protected:
    virtual void LevelExit();
    virtual void MtrlParamUpdate();

public:
    void TurnOnBossHP();
    void TurnOffBossHP();
    void BossRevive();

    void TurnOnPlayerHP();
    void TurnOffPlayerHP();

    void TurnOffStageClearUI();
    void TurnOnStageclearUI();

    void LevelLoading();
    void SetEnterTime(const float _fTime) { m_fFadeInWaitTime = _fTime; }

    void SetUIDOFEffect();

    void TurnOffDropUI();
    void TurnOnDropUI();

    void ActiveOffDropUI();
    void ActiveOnDropUI();

    void ContinueUIOn();
    void ContinueUIOff();

    void AbsorbUIOn(CGameObject* _pObj);
    void AbsorbUIOff();
    void AbsorbUIEndOff();

public:
    void ResetFadeEffectTimer();

public:
    virtual UINT SaveToLevelFile(FILE* _File) override;
    virtual UINT LoadFromLevelFile(FILE* _File) override;

public:
    virtual CLevelFlowMgr* Clone() = 0;
    CLevelFlowMgr(UINT _Type);
    CLevelFlowMgr(const CLevelFlowMgr& _Origin);
    virtual ~CLevelFlowMgr();
};
