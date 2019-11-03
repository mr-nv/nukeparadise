
#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"


class CBaseAnimating
{
public:
    std::array<float, 24>* m_flPoseParameter()
    {
        static int offset = 0;

        if ( !offset )
        {
            offset = 0x2764;
        }

        return ( std::array<float, 24>* ) ( ( uintptr_t ) this + offset );
    }
    model_t* GetModel()
    {
        void* pRenderable = reinterpret_cast<void*> ( uintptr_t ( this ) + 0x4 );
        typedef model_t* ( __thiscall * fnGetModel ) ( void* );
        return CallVFunction<fnGetModel> ( pRenderable, 8 ) ( pRenderable );
    }
    void SetBoneMatrix ( matrix3x4_t* boneMatrix )
    {
        //Offset found in C_BaseAnimating::GetBoneTransform, string search ankle_L and a function below is the right one
        const auto model = this->GetModel();

        if ( !model )
        {
            return;
        }

        matrix3x4_t* matrix = * ( matrix3x4_t** ) ( ( DWORD ) this + 9880 );
        studiohdr_t* hdr = g_MdlInfo->GetStudiomodel ( model );

        if ( !hdr )
        {
            return;
        }

        int size = hdr->numbones;

        if ( matrix )
        {
            for ( int i = 0; i < size; i++ )
            {
                memcpy ( matrix + i, boneMatrix + i, sizeof ( matrix3x4_t ) );
            }
        }
    }
    void GetDirectBoneMatrix ( matrix3x4_t* boneMatrix )
    {
        const auto model = this->GetModel();

        if ( !model )
        {
            return;
        }

        matrix3x4_t* matrix = * ( matrix3x4_t** ) ( ( DWORD ) this + 9880 );
        studiohdr_t* hdr = g_MdlInfo->GetStudiomodel ( model );

        if ( !hdr )
        {
            return;
        }

        int size = hdr->numbones;

        if ( matrix )
        {
            for ( int i = 0; i < size; i++ )
            {
                memcpy ( boneMatrix + i, matrix + i, sizeof ( matrix3x4_t ) );
            }
        }
    }
};

#pragma once
class ThirdpersonAngleHelper : public Singleton<ThirdpersonAngleHelper>
{
public:
	void re_work();
	void fix_local_player_animations();
	void update_animations(C_BasePlayer* entity);
	void AnimFix();
    void AnimFix2 ( C_BasePlayer* entity );
    void SetThirdpersonAngle();
    void EnemyAnimationFix ( C_BasePlayer* player );
private:
    QAngle LastAngle = QAngle ( 0, 0, 0 );
};

