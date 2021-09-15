#pragma once

#include "AzureKinectEnum.generated.h"

/**
 * Blueprintable enum defined based on k4a_depth_mode_t from k4atypes.h
 *
 * @note This should always have the same enum values as k4a_depth_mode_t
 */
UENUM(BlueprintType, Category = "Azure Kinect|Enums")
enum class EKinectDepthMode : uint8
{
	OFF = 0         UMETA(DisplayName = "Depth Mode Off"),				/**< Depth sensor will be turned off with this setting. */
	NFOV_2X2BINNED  UMETA(DisplayName = "NFOV 2x2 Binned (320x288)"),	/**< Depth captured at 320x288. Passive IR is also captured at 320x288. */
	NFOV_UNBINNED   UMETA(DisplayName = "NFOV Unbinned (640x576)"),		/**< Depth captured at 640x576. Passive IR is also captured at 640x576. */
	WFOV_2X2BINNED  UMETA(DisplayName = "WFOV 2x2 Binned (512x512)"),	/**< Depth captured at 512x512. Passive IR is also captured at 512x512. */
	WFOV_UNBINNED   UMETA(DisplayName = "WFOV Unbinned (1024x1024)"),	/**< Depth captured at 1024x1024. Passive IR is also captured at 1024x1024. */
	PASSIVE_IR      UMETA(DisplayName = "Passive IR (1024x1024)"),		/**< Passive IR only, captured at 1024x1024. */
};

UENUM(BlueprintType, Category = "Azure Kinect|Enums")
enum class EKinectColorResolution : uint8
{
	RESOLUTION_OFF = 0		UMETA(DisplayName = "Color Camera Turned Off"),		/**< Color sensor will be turned off with this setting. */
	RESOLUTION_720P			UMETA(DisplayName = "1280 x 720  [16:9]"),		/**< Color captured at 1280 x 720. */
	RESOLUTION_1440P		UMETA(DisplayName = "2560 x 1440 [16:9]"),		/**< Color captured at 2560 x 1440. */
	RESOLUTION_1536P		UMETA(DisplayName = "2048 x 1536 [4:3]"),		/**< Color captured at 2048 x 1536. */
	RESOLUTION_2160P		UMETA(DisplayName = "3840 x 2160 [16:9]"),		/**< Color captured at 3840 x 2160. */
	RESOLUTION_3072P		UMETA(DisplayName = "4096 x 3072 [4:3]"),		/**< Color captured at 4096 x 3072. */
};

UENUM(BlueprintType, Category = "Azure Kinect|Enums")
enum class EKinectFps : uint8
{
	PER_SECOND_5 = 0	UMETA(DisplayName = "5 fps"),
	PER_SECOND_15		UMETA(DisplayName = "15 fps"),
	PER_SECOND_30		UMETA(DisplayName = "30 fps"),
};

/**
 * Blueprintable enum defined based on k4abt_joint_id_t from k4abttypes.h
 * This should always have the same enum values as k4abt_joint_id_t
 */
UENUM(BlueprintType, Category = "Azure Kinect|Enums")
enum class EKinectBodyJoint : uint8
{
	PELVIS = 0      UMETA(DisplayName = "Pelvis"),
	SPINE_NAVEL     UMETA(DisplayName = "Spine Navel"),
	SPINE_CHEST     UMETA(DisplayName = "Spine Chest"),
	NECK            UMETA(DisplayName = "Neck"),
	CLAVICLE_LEFT   UMETA(DisplayName = "Clavicle Left"),
	SHOULDER_LEFT   UMETA(DisplayName = "Shoulder Left"),
	ELBOW_LEFT      UMETA(DisplayName = "Elbow Left"),
	WRIST_LEFT      UMETA(DisplayName = "Wrist Left"),
	HAND_LEFT       UMETA(DisplayName = "Hand Left"),
	HANDTIP_LEFT    UMETA(DisplayName = "Hand Tip Left"),
	THUMB_LEFT      UMETA(DisplayName = "Thumb Left"),
	CLAVICLE_RIGHT  UMETA(DisplayName = "Clavicle Right"),
	SHOULDER_RIGHT  UMETA(DisplayName = "Shoulder Right"),
	ELBOW_RIGHT     UMETA(DisplayName = "Elbow Right"),
	WRIST_RIGHT     UMETA(DisplayName = "Wrist Right"),
	HAND_RIGHT      UMETA(DisplayName = "Hand Right"),
	HANDTIP_RIGHT   UMETA(DisplayName = "Hand Tip Right"),
	THUMB_RIGHT     UMETA(DisplayName = "Thumb Right"),
	HIP_LEFT        UMETA(DisplayName = "Hip Left"),
	KNEE_LEFT       UMETA(DisplayName = "Knee Left"),
	ANKLE_LEFT      UMETA(DisplayName = "Ankle Left"),
	FOOT_LEFT       UMETA(DisplayName = "Foot Left"),
	HIP_RIGHT       UMETA(DisplayName = "Hip Right"),
	KNEE_RIGHT      UMETA(DisplayName = "Knee Right"),
	ANKLE_RIGHT     UMETA(DisplayName = "Ankle Right"),
	FOOT_RIGHT      UMETA(DisplayName = "Foot Right"),
	HEAD            UMETA(DisplayName = "Head"),
	NOSE            UMETA(DisplayName = "Nose"),
	EYE_LEFT        UMETA(DisplayName = "Eye Left"),
	EAR_LEFT        UMETA(DisplayName = "Ear Left"),
	EYE_RIGHT       UMETA(DisplayName = "Eye Right"),
	EAR_RIGHT       UMETA(DisplayName = "Ear Right"),
	COUNT           UMETA(DisplayName = "COUNT", Hidden),
};

/**
* This should always have the same enum values as k4abt_sensor_orientation_t
*/
UENUM(BlueprintType, Category = "Azure Kinect|Enums")
enum class EKinectSensorOrientation : uint8
{
	DEFAULT = 0			UMETA(DisplayName = "Default"), /**< Mount the sensor at its default orientation */
	CLOCKWISE90			UMETA(DisplayName = "Clockwise 90"), /**< Clockwisely rotate the sensor 90 degree */
	COUNTERCLOCKWISE90	UMETA(DisplayName = "Conter-clockwise 90"), /**< Counter-clockwisely rotate the sensor 90 degrees */
	FLIP180				UMETA(DisplayName = "Flip 180"), /**< Mount the sensor upside-down */
};