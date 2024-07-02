#ifndef _VIEWER_CAMERA_H_
#define _VIEWER_CAMERA_H_


// I N C L U D E S /////////////////////////////////////////////////


// D E F I N E S ///////////////////////////////////////////////////


// S T R U C T S ///////////////////////////////////////////////////

namespace VIEWER {

class Camera
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	cv::Size size;
	AABB3d boxScene;
	Eigen::Vector3d centerScene;
	Eigen::Quaterniond rotation;
	Eigen::Vector3d center;
	double dist, radius;
	float fov, fovDef;
	float scaleF, scaleFDef;
	MVS::IIndex prevCamID, currentCamID, maxCamID;

public:
	Camera(const AABB3d& _box=AABB3d(true), const Point3d& _center=Point3d::ZERO, float _scaleF=1, float _fov=40);

	void Reset();
	void Resize(const cv::Size&);
	void SetFOV(float _fov);

	const cv::Size& GetSize() const { return size; }

	Eigen::Vector3d GetPosition() const;
	Eigen::Matrix3d GetRotation() const;
	Eigen::Matrix4d GetLookAt() const;

	void GetLookAt(Eigen::Vector3d& eye, Eigen::Vector3d& center, Eigen::Vector3d& up) const;
	void Rotate(const Eigen::Vector2d& pos, const Eigen::Vector2d& prevPos);
	void Translate(const Eigen::Vector2d& pos, const Eigen::Vector2d& prevPos);

	bool IsCameraViewMode() const { return prevCamID != currentCamID && currentCamID != NO_ID; }

protected:
	void ProjectOnSphere(double radius, Eigen::Vector3d& p) const;
};
/*----------------------------------------------------------------*/

} // namespace VIEWER

#endif // _VIEWER_CAMERA_H_
