#include "Camera.hpp"
#include <iostream>

namespace gps {

	//Camera constructor
	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;

		this->cameraUpDirection = cameraUp;
		this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
		this->cameraRightDirection = glm::cross(this->cameraFrontDirection, this->cameraUpDirection);

		//TODO - Update the rest of camera parameters

	}

	//return the view matrix, using the glm::lookAt() function
	glm::mat4 Camera::getViewMatrix() {
		return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
	}

	//update the camera internal parameters following a camera move event
	void Camera::move(MOVE_DIRECTION direction, float speed) {
		//TODO
		switch (direction) {
		case MOVE_FORWARD: cameraPosition += cameraFrontDirection * speed;
			cameraTarget += cameraFrontDirection * speed;
			break;
		case MOVE_BACKWARD: cameraPosition -= cameraFrontDirection * speed;
			cameraTarget -= cameraFrontDirection * speed;
			break;
		case MOVE_LEFT: 
			cameraPosition -= cameraRightDirection * speed;
			cameraTarget -= cameraRightDirection * speed;
			break;
		case MOVE_RIGHT: 
			cameraPosition += cameraRightDirection * speed;
			cameraTarget += cameraRightDirection * speed;
			break;
		case MOVE_UP: 
			cameraPosition += cameraUpDirection * speed;
			cameraTarget += cameraUpDirection * speed;
			break;
		case MOVE_DOWN: 
			cameraPosition -= cameraUpDirection * speed;
			cameraTarget -= cameraUpDirection * speed;
		default: break;
		}
		
		this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
		this->cameraRightDirection = glm::cross(this->cameraFrontDirection, this->cameraUpDirection);
	}


	//update the camera internal parameters following a camera rotate event
	//yaw - camera rotation around the y axis
	//pitch - camera rotation around the x axis
	void Camera::rotate(float pitch, float yaw) {
		//TODO
		this->yawAngle = this->yawAngle + yaw;
		this->pitchAngle = this->pitchAngle + pitch;
		this->cameraTarget.x = this->cameraPosition.x + sin(this->yawAngle);
		this->cameraTarget.y = this->cameraPosition.y + sin(this->pitchAngle);
		this->cameraTarget.z = this->cameraPosition.z - cos(yawAngle);
		this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
		this->cameraRightDirection = glm::cross(this->cameraFrontDirection, this->cameraUpDirection);
	}

	glm::vec3 Camera::getCameraPosition() {
		return this->cameraPosition;
	}

	glm::vec3 Camera::getCameraTarget() {
		return this->cameraTarget;
	}

	void Camera::setCamera(glm::vec3 pos, glm::vec3 target, glm::vec3 upDir) {
		this->cameraPosition = pos;
		this->cameraTarget = target;
		this->cameraUpDirection = upDir;
		this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
		this->cameraRightDirection = glm::cross(this->cameraFrontDirection, this->cameraUpDirection);
	}
}