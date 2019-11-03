//nauka ob otnosheniyah mezhdu obektami, o kotoryh nichego ne izvestno, krome...
#include "../valve_sdk/csgostructs.hpp"
#include "Math.hpp"
#include <algorithm>

namespace Math
{
	inline Vector CrossProduct(const Vector& a, const Vector& b)
	{
		return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}
	void correct_movement(CUserCmd* cmd, const QAngle& old_angles)
	{
		CorrectMovement(old_angles, cmd, cmd->forwardmove, cmd->sidemove);
	}
	void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles) {
		if (old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll) {
			Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

			auto viewangles = old_angles;
			auto movedata = Vector(m_Cmd->forwardmove, m_Cmd->sidemove, m_Cmd->upmove);
			viewangles.Normalize();

			if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
				movedata.y = 0.f;

			AngleVectors(wish_angle, wish_forward, wish_right, wish_up);
			AngleVectors(viewangles, cmd_forward, cmd_right, cmd_up);

			auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

			Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
				wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
				wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

			auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

			Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
				cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
				cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

			auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

			Vector correct_movement;
			correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
			correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
			correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

			correct_movement.x = std::clamp(correct_movement.x, -450.f, 450.f);
			correct_movement.y = std::clamp(correct_movement.y, -450.f, 450.f);
			correct_movement.z = std::clamp(correct_movement.z, -320.f, 320.f);

			m_Cmd->forwardmove = correct_movement.x;
			m_Cmd->sidemove = correct_movement.y;
			m_Cmd->upmove = correct_movement.z;

			m_Cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
			if (m_Cmd->sidemove != 0.0) {
				if (m_Cmd->sidemove <= 0.0)
					m_Cmd->buttons |= IN_MOVELEFT;
				else
					m_Cmd->buttons |= IN_MOVERIGHT;
			}

			if (m_Cmd->forwardmove != 0.0) {
				if (m_Cmd->forwardmove <= 0.0)
					m_Cmd->buttons |= IN_BACK;
				else
					m_Cmd->buttons |= IN_FORWARD;
			}
		}
	}
    //--------------------------------------------------------------------------------
    float VectorDistance(const Vector& v1, const Vector& v2)
    {
        return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
    }
    //--------------------------------------------------------------------------------
	void FixAngles(QAngle& angles)
	{
		lNormalizeAngles(angles);
		lClampAngles(angles);
	}
	void lNormalizeAngles(QAngle& angle)
	{
		while (angle.pitch > 89.0f)
			angle.pitch -= 180.f;

		while (angle.pitch < -89.0f)
			angle.pitch += 180.f;

		while (angle.yaw > 180.f)
			angle.yaw -= 360.f;

		while (angle.yaw < -180.f)
			angle.yaw += 360.f;
	}
	void lClampAngles(QAngle& angles)
	{
		if (angles.pitch > 89.0f) angles.pitch = 89.0f;
		else if (angles.pitch < -89.0f) angles.pitch = -89.0f;

		if (angles.yaw > 180.0f) angles.yaw = 180.0f;
		else if (angles.yaw < -180.0f) angles.yaw = -180.0f;

		angles.roll = 0;
	}
	//--------------------------------------------------------------------------------
	bool IntersectionBoundingBox(const Vector& src, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point)
	{
		/*
		Fast Ray-Box Intersection
		by Andrew Woo
		from "Graphics Gems", Academic Press, 1990
		*/

		constexpr auto NUMDIM = 3;
		constexpr auto RIGHT = 0;
		constexpr auto LEFT = 1;
		constexpr auto MIDDLE = 2;

		bool inside = true;
		char quadrant[NUMDIM];
		int i;

		// Rind candidate planes; this loop can be avoided if
		// rays cast all from the eye(assume perpsective view)
		Vector candidatePlane;
		for (i = 0; i < NUMDIM; i++) {
			if (src[i] < min[i]) {
				quadrant[i] = LEFT;
				candidatePlane[i] = min[i];
				inside = false;
			}
			else if (src[i] > max[i]) {
				quadrant[i] = RIGHT;
				candidatePlane[i] = max[i];
				inside = false;
			}
			else {
				quadrant[i] = MIDDLE;
			}
		}

		// Ray origin inside bounding box
		if (inside) {
			if (hit_point)
				* hit_point = src;
			return true;
		}

		// Calculate T distances to candidate planes
		Vector maxT;
		for (i = 0; i < NUMDIM; i++) {
			if (quadrant[i] != MIDDLE && dir[i] != 0.f)
				maxT[i] = (candidatePlane[i] - src[i]) / dir[i];
			else
				maxT[i] = -1.f;
		}

		// Get largest of the maxT's for final choice of intersection
		int whichPlane = 0;
		for (i = 1; i < NUMDIM; i++) {
			if (maxT[whichPlane] < maxT[i])
				whichPlane = i;
		}

		// Check final candidate actually inside box
		if (maxT[whichPlane] < 0.f)
			return false;

		for (i = 0; i < NUMDIM; i++) {
			if (whichPlane != i) {
				float temp = src[i] + maxT[whichPlane] * dir[i];
				if (temp < min[i] || temp > max[i]) {
					return false;
				}
				else if (hit_point) {
					(*hit_point)[i] = temp;
				}
			}
			else if (hit_point) {
				(*hit_point)[i] = candidatePlane[i];
			}
		}

		// ray hits box
		return true;
	}
	//--------------------------------------------------------------------------------
    void NormalizeAngles(QAngle& angles)
    {
        for (auto i = 0; i < 3; i++)
        {
            while (angles[i] < -180.0f)
            {
                angles[i] += 360.0f;
            }
            while (angles[i] > 180.0f)
            {
                angles[i] -= 360.0f;
            }
        }
    }
    void NormalizePitch(float& pitch)
    {
        while (pitch < -180.0f)
        {
            pitch += 360.0f;
        }
        while (pitch > 180.0f)
        {
            pitch -= 360.0f;
        }
    }
    int RandomInt(int min, int max)
    {
        return rand() % (max - min + 1) + min;
    }
    QAngle CalcAngle(const Vector& src, const Vector& dst)
    {
        QAngle vAngle;
        Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
        double hyp = sqrt(delta.x*delta.x + delta.y*delta.y);

        vAngle.pitch = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
        vAngle.yaw = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
        vAngle.roll = 0.0f;

        if (delta.x >= 0.0)
        {
            vAngle.yaw += 180.0f;
        }

        return vAngle;
    }

	Vector gCalcAngle(Vector src, Vector dst)
	{
		auto ret = Vector();
		auto delta = src - dst;
		double hyp = delta.Length2D();
		ret.y = (atan(delta.y / delta.x) * 57.295779513082f);
		ret.x = (atan(delta.z / hyp) * 57.295779513082f);
		ret[2] = 0.00;

		if (delta.x >= 0.0)
			ret.y += 180.0f;

		return ret;
	}

    Vector CrossProduct2(const Vector& a, const Vector& b)
    {
        return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
    }
    void VectorAngles(const Vector& forward, Vector& up, QAngle& angles)
    {
        Vector left = CrossProduct2(up, forward);
        left.NormalizeInPlace();

        float forwardDist = forward.Length2D();

        if (forwardDist > 0.001f)
        {
            angles.pitch = atan2f(-forward.z, forwardDist) * 180 / PI_F;
            angles.yaw = atan2f(forward.y, forward.x) * 180 / PI_F;

            float upZ = (left.y * forward.x) - (left.x * forward.y);
            angles.roll = atan2f(left.z, upZ) * 180 / PI_F;
        }
        else
        {
            angles.pitch = atan2f(-forward.z, forwardDist) * 180 / PI_F;
            angles.yaw = atan2f(-left.x, left.y) * 180 / PI_F;
            angles.roll = 0;
        }
    }
    float RandomFloat(float min, float max)
    {
        static auto ranFloat = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandleW(L"vstdlib.dll"), "RandomFloat"));
        if (ranFloat)
        {
            return ranFloat(min, max);
        }
        else
        {
            return 0.f;
        }
    }
	void RandomSeed(int iSeed)
	{
		static auto ranSeed = reinterpret_cast<void(*)(int)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed"));
		if (ranSeed)
			ranSeed(iSeed);
	}
    //--------------------------------------------------------------------------------
    float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle)
    {
        Vector ang, aim;

        AngleVectors(viewAngle, aim);
        AngleVectors(aimAngle, ang);

        return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
    }
    void AngleVector(const Vector& angles, Vector& forward)
    {
        Assert(s_bMathlibInitialized);
        Assert(forward);

        float sp, sy, cp, cy;

        Math::SinCos(DEG2RAD(angles.y), &sy, &cy);
        Math::SinCos(DEG2RAD(angles.x), &sp, &cp);

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
    }
    //--------------------------------------------------------------------------------
    void ClampAngles(QAngle& angles)
    {
        if (angles.pitch > 89.0f)
        {
            angles.pitch = 89.0f;
        }
        else if (angles.pitch < -89.0f)
        {
            angles.pitch = -89.0f;
        }

        if (angles.yaw > 180.0f)
        {
            angles.yaw = 180.0f;
        }
        else if (angles.yaw < -180.0f)
        {
            angles.yaw = -180.0f;
        }

        angles.roll = 0;
    }
    //--------------------------------------------------------------------------------
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
    {
        //auto t = in2[0];
        out[0] = in1.Dot(in2[0]) + in2[0][3];
        out[1] = in1.Dot(in2[1]) + in2[1][3];
        out[2] = in1.Dot(in2[2]) + in2[2][3];
    }
	//--------------------------------------------------------------------------------
	void Normalize(Vector& f)
	{
		while (f.y <= -180) f.y += 360;
		while (f.y > 180) f.y -= 360;
		while (f.x <= -180) f.x += 360;
		while (f.x > 180) f.x -= 360;


		if (f.x > 89.0) f.x = 89;
		if (f.x < -89.0) f.x = -89;
		if (f.y < -180) f.y = -179.999;
		if (f.y > 180) f.y = 179.999;
	}
	//---------------------------------------------------------------------------------
	void CorrectMovement(QAngle vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove)
	{
		// side/forward move correction
		float deltaView;
		float f1;
		float f2;

		if (vOldAngles.yaw < 0.f)
			f1 = 360.0f + vOldAngles.yaw;
		else
			f1 = vOldAngles.yaw;

		if (pCmd->viewangles.yaw < 0.0f)
			f2 = 360.0f + pCmd->viewangles.yaw;
		else
			f2 = pCmd->viewangles.yaw;

		if (f2 < f1)
			deltaView = abs(f2 - f1);
		else
			deltaView = 360.0f - abs(f1 - f2);

		deltaView = 360.0f - deltaView;

		pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
		pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
	}
	//--------------------------------------------------------------------------------
	void gAngleVectors(const Vector& angles, Vector& forward)
	{
		float sp, sy, cp, cy;
		SinCos(DEG2RAD(angles[1]), &sy, &cy);
		SinCos(DEG2RAD(angles[0]), &sp, &cp);
		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;
	}
	//--------------------------------------------------------------------------------
	void gVectorAngles(const Vector& forward, Vector& up, Vector& angles)
	{
		Vector left = CrossProduct(up, forward);
		left.NormalizeInPlace();
		float forwardDist = forward.Length2D();

		if (forwardDist > 0.001f)
		{
			angles.x = atan2f(-forward.z, forwardDist) * 180 / M_PI;
			angles.y = atan2f(forward.y, forward.x) * 180 / M_PI;
			float upZ = (left.y * forward.x) - (left.x * forward.y);
			angles.z = atan2f(left.z, upZ) * 180 / M_PI;
		}

		else
		{
			angles.x = atan2f(-forward.z, forwardDist) * 180 / M_PI;
			angles.y = atan2f(-left.x, left.y) * 180 / M_PI;
			angles.z = 0;
		}
	}
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle& angles, Vector& forward)
    {
        float	sp, sy, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));

        forward.x = cp*cy;
        forward.y = cp*sy;
        forward.z = -sp;
    }
	//--------------------------------------------------------------------------------
	void gVectorAngles(const Vector& forward, Vector& angles)
	{
		float tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0)
		{
			yaw = 0;

			if (forward[2] > 0)
				pitch = 270;

			else
				pitch = 90;
		}

		else
		{
			yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);

			if (yaw < 0)
				yaw += 360;

			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / M_PI);

			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle& angles, Vector& forward, Vector& right, Vector& up)
    {
        float sr, sp, sy, cr, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));
        DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles[2]));

        forward.x = (cp * cy);
        forward.y = (cp * sy);
        forward.z = (-sp);
        right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
        right.z = (-1 * sr * cp);
        up.x = (cr * sp * cy + -sr*-sy);
        up.y = (cr * sp * sy + -sr*cy);
        up.z = (cr * cp);
    }
    //--------------------------------------------------------------------------------
    void VectorAngles(const Vector& forward, QAngle& angles)
    {
        float	tmp, yaw, pitch;

        if(forward[1] == 0 && forward[0] == 0)
        {
            yaw = 0;
            if(forward[2] > 0)
            {
                pitch = 270;
            }
            else
            {
                pitch = 90;
            }
        }
        else
        {
            yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);
            if(yaw < 0)
            {
                yaw += 360;
            }

            tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);
            if(pitch < 0)
            {
                pitch += 360;
            }
        }

        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0;
    }
    //--------------------------------------------------------------------------------
    static bool screen_transform(const Vector& in, Vector& out)
    {
        static auto& w2sMatrix = g_EngineClient->WorldToScreenMatrix();

        out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
        out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
        out.z = 0.0f;

        float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

        if(w < 0.001f)
        {
            out.x *= 100000;
            out.y *= 100000;
            return false;
        }

        out.x /= w;
        out.y /= w;

        return true;
    }
    //--------------------------------------------------------------------------------
    bool WorldToScreen(const Vector& in, Vector& out)
    {
        if(screen_transform(in, out))
        {
            int w, h;
            g_EngineClient->GetScreenSize(w, h);

            out.x = (w / 2.0f) + (out.x * w) / 2.0f;
            out.y = (h / 2.0f) - (out.y * h) / 2.0f;

            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------------
	QAngle calculate_angle(Vector src, Vector dst) {
		QAngle angles;

		Vector delta = src - dst;
		float hyp = delta.Length2D();

		angles.yaw = std::atanf(delta.y / delta.x) * 57.2957795131f;
		angles.pitch = std::atanf(-delta.z / hyp) * -57.2957795131f;
		angles.roll = 0.0f;

		if (delta.x >= 0.0f)
			angles.yaw += 180.0f;

		return angles;
	}

	//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\

	void VECTOR_Normalize(Vector& vIn, Vector& vOut)
	{
		float flLen = vIn.Length();
		if (flLen == 0) {
			vOut.Init(0, 0, 1);
			return;
		}
		flLen = 1 / flLen;
		vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
	}

	void QANGLE_Normalize(QAngle& vIn, QAngle& vOut)
	{
		float flLen = vIn.Length();
		if (flLen == 0) {
			vOut.Init(0, 0, 1);
			return;
		}
		flLen = 1 / flLen;
		vOut.Init(vIn.pitch * flLen, vIn.yaw * flLen, vIn.roll * flLen);
	}

	void VECTOR_NormaliseViewAngle(Vector& angle)
	{		
			while (angle.y <= -180) angle.y += 360;
			while (angle.y > 180) angle.y -= 360;
			while (angle.x <= -180) angle.x += 360;
			while (angle.x > 180) angle.x -= 360;


			if (angle.x > 89) angle.x = 89;
			if (angle.x < -89) angle.x = -89;
			if (angle.y < -180) angle.y = -179.999;
			if (angle.y > 180) angle.y = 179.999;

			angle.z = 0;
	}

	void QANGLE_NormaliseViewAngle(QAngle& angle)
	{
			while (angle.yaw <= -180) angle.yaw += 360;
			while (angle.yaw > 180) angle.yaw -= 360;
			while (angle.pitch <= -180) angle.pitch += 360;
			while (angle.pitch > 180) angle.pitch -= 360;


			if (angle.pitch > 89) angle.pitch = 89;
			if (angle.pitch < -89) angle.pitch = -89;
			if (angle.yaw < -180) angle.yaw = -179.999;
			if (angle.yaw > 180) angle.yaw = 179.999;

			angle.roll = 0;
	}

	//Thrill Pill

	QAngle VECTOR_TO_QANGLE(Vector& angle)
	{
		Vector angles;
		QAngle result;

		angles.x = result.pitch;
		angles.y = result.yaw;
		angles.z = result.roll;

		return result;

	}

	Vector QANGLE_TO_VECTOR(QAngle& angle)
	{
		QAngle angles;
		Vector result;

		angles.pitch = result.x;
		angles.yaw = result.y;
		angles.roll = result.z;

		return result;
	}

}