#pragma once

#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include <memory>
#include <concepts>

using namespace DirectX;
using Mat4 = XMFLOAT4X4;
using Mat = XMMATRIX;


enum class ProjectionType { ORTHOGRAPHIC, PERSPECTIVE };

struct Projection {

	Mat projMat{};
	ProjectionType type{ProjectionType::PERSPECTIVE};
	[[nodiscard]] virtual Mat getProjMatrix() const noexcept { return projMat; }
	virtual void updateProjection() noexcept = 0;

	Projection() = default;
	Projection(const Projection&) = default;
	Projection& operator=(const Projection&) = default;
	Projection(Projection&&) = default;
	Projection& operator=(Projection&&) = default;
	virtual ~Projection() = default;
};

struct PerspectiveProjection : Projection
{
	static float s_baseAspectRatio;
	float aspectRatio = 16.f/9.f;
	float fov = 3.141592653f / 4.f;
	float znear = 0.10f, zfar = 1000.F;

	PerspectiveProjection()
	{
		aspectRatio = s_baseAspectRatio;
		type = ProjectionType::PERSPECTIVE;
		projMat = XMMatrixPerspectiveFovLH(fov, aspectRatio, znear, zfar);
	}

	PerspectiveProjection(
		const float aspect, 
		const float fov ,
		const float zn, 
		const float zf)
	{
		type = ProjectionType::PERSPECTIVE;
		projMat = XMMatrixPerspectiveFovLH(fov, aspectRatio, znear, zfar);
	}
	~PerspectiveProjection() override = default;
	virtual void updateProjection() noexcept override final {
		projMat = XMMatrixPerspectiveFovLH(fov, aspectRatio, znear, zfar);
	}
};


struct OrthographicProjection :  Projection 
{
	float left, right;
	float top, bot;
	float znear, zfar;

	OrthographicProjection(
		const float l, 	const float r, 
		const float t, 	const float b, 
		const float zn,	const float zf)
			: left (l)	, right (r)
			, top (t)	, bot (b)
			, znear (zn), zfar (zf)

	{
		type = ProjectionType::ORTHOGRAPHIC;
		projMat = XMMatrixOrthographicOffCenterLH(left, right, bot, top, znear, zfar);
	}


	[[nodiscard]]  Mat getProjMatrix() const noexcept override final 
	{
		return XMMatrixOrthographicOffCenterLH(left,right, top, bot,znear,zfar);
	}
	virtual void updateProjection() noexcept override final {
		projMat = XMMatrixOrthographicOffCenterLH(left, right, top, bot, znear, zfar);
	}
	~OrthographicProjection() override = default;
};


class Camera
{
private:

	XMVECTOR m_position { 0.f,0.f,0.0f,1.0f };
	XMVECTOR m_up		{ 0.f,1.f,0.f,1.f };
	XMVECTOR m_left		{ 1.f,0.f,0.f,1.f };

	std::unique_ptr<Projection> m_projection;

	struct Angles {
		float yaw = 0.F;
		float pitch = 0.F;
		float roll = 0.f;

		[[nodiscard]] XMVECTOR toVec() const { return XMVECTOR{
			-pitch ,
			-yaw,
			-roll };
		}


	} m_angles;

	Mat m_viewProjMatrix{};
	Mat m_viewMatrix{};

	float m_rotationSpeed = 1.f;


public:

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Basic operations

	void updateCam();
	void move(const XMVECTOR& delta);
	void setPosition(const XMVECTOR& pos);
	void rotate(float dx = 0.0F, float dy = 0.0F, float dz = 0.0F);
	void lookAt(const XMVECTOR& target);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Matrices and projection


	void computeVPMatrix();
	void computeViewMatrix();

	[[nodiscard]] Mat getVPMatrix();
	[[nodiscard]] Mat getVPMatrix() const ;
	[[nodiscard]] Mat getViewMatrix() const { return m_viewMatrix; }
	[[nodiscard]] Mat getProjMatrix() const;

	void setFieldOfView(float fov) {
		if (m_projection->type != ProjectionType::PERSPECTIVE) return;
		auto p = reinterpret_cast<PerspectiveProjection*>(m_projection.get());
		p->fov = fov;
		p->updateProjection();
		
	}

	template<class _proj = PerspectiveProjection>
		requires (std::derived_from<_proj, Projection>)
	void setProjection(const _proj& proj)
	{
		m_projection = std::make_unique<_proj>(proj);
	}
	template<class _proj = PerspectiveProjection>
		requires (std::derived_from<_proj, Projection>)
	[[nodiscard]] const _proj& getProjection() const noexcept { return *reinterpret_cast<_proj*>(m_projection.get()); }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Directions and position

	[[nodiscard]] XMVECTOR getPosition() const noexcept;
	[[nodiscard]] XMVECTOR getUp() const noexcept;
	[[nodiscard]] XMVECTOR getForward() const  noexcept;
	[[nodiscard]] XMVECTOR getHorizontalDir() const noexcept;
	[[nodiscard]] XMVECTOR getForwardDir() const noexcept;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Angles

	void setYaw(float yaw)								{ m_angles.yaw = yaw;		}
	void setPitch(float pitch)							{ m_angles.pitch = pitch;	}
	void setRoll(float roll)							{ m_angles.roll = roll;		}
	[[nodiscard]] XMVECTOR getAngles()	const noexcept	{ return m_angles.toVec();	}
	[[nodiscard]] float getPitch()		const noexcept	{ return m_angles.pitch;	}
	[[nodiscard]] float getYaw()		const noexcept	{ return m_angles.yaw;		}
	[[nodiscard]] float getRoll()		const noexcept	{ return m_angles.roll;		}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Constructors, move and stuff

	void swap(Camera& other) noexcept;
	Camera() = default;
	Camera(const Camera& other);
	Camera& operator=(const Camera& other);
	Camera(Camera&& other) noexcept;
	Camera& operator=(Camera&& other) noexcept;
	~Camera() = default;
};

