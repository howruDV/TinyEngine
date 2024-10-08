#pragma once
#include "CComputeShader.h"

class CAnimationUpdateShader : public CComputeShader
{
private:
    CStructuredBuffer* m_pFrameDataBuffer; // t32
    CStructuredBuffer* m_pOffsetMatBuffer; // t33

    CStructuredBuffer* m_pBoneTransformMatBuffer; // u0
    CStructuredBuffer* m_pFinalMatBuffer;         // u1
    CStructuredBuffer* m_pPrevFinalMatBuffer;     // u2

public:
    // g_int_0 : BonCount, g_int_1 : Frame Index
    void SetBoneCount(int _iBoneCount) { m_Const.arrInt[0] = _iBoneCount; }
    void SetFrameIndex(int _iFrameIdx) { m_Const.arrInt[1] = _iFrameIdx; }
    void SetNextFrameIdx(int _iFrameIdx) { m_Const.arrInt[2] = _iFrameIdx; }
    void SetFrameRatio(float _fFrameRatio) { m_Const.arrFloat[0] = _fFrameRatio; }

    void SetFrameDataBuffer(CStructuredBuffer* _buffer) { m_pFrameDataBuffer = _buffer; }
    void SetOffsetMatBuffer(CStructuredBuffer* _buffer) { m_pOffsetMatBuffer = _buffer; }
    void SetBoneTransformMatBuffer(CStructuredBuffer* _buffer) { m_pBoneTransformMatBuffer = _buffer; }
    void SetFinalMatBuffer(CStructuredBuffer* _buffer) { m_pFinalMatBuffer = _buffer; }
    void SetPrevFinalMatBuffer(CStructuredBuffer* _buffer) { m_pPrevFinalMatBuffer = _buffer; }

public:
    virtual int UpdateData() override;
    virtual void UpdateGroupCount() override;
    virtual void Clear() override;

public:
    CAnimationUpdateShader();
    virtual ~CAnimationUpdateShader();
};
