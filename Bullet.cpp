#include"Bullet.h"
#

//Only thing that constructor does is it sets the size of the bullet (RectangleShape object)
Bullet::Bullet() {
	m_BulletShape.setSize(sf::Vector2f(2, 2));
}
void Bullet::shoot(float startX, float startY, float targetX, float targetY) {
	m_InFlight = true;
	m_Position.x = startX;
	m_Position.y = startY;
	//Calculate gradient of the flight path 
	float gradient = (startX - targetX) / (startY - targetY);
	if (gradient < 0)
	{
		gradient *= -1;
	}
	//Calculate ratio between x and y - change the bullet's h and v position by the correct amount each frame
	float ratioXY = m_BulletSpeed / (1 + gradient);
	//Set the "speed" horizontally and vertically
	m_BulletDistanceY = ratioXY;
	m_BulletDistanceX = ratioXY * gradient;
	//Point the bullet in the right direction
	if (targetX < startX)
	{
		m_BulletDistanceX *= -1;
	}
	if (targetY < startY)
	{
		m_BulletDistanceY *= -1;
	}
	//Set a max range of 1000 pixels
	float range = 1000;
	m_MinX = startX - range;
	m_MaxX = startX + range;
	m_MinY = startY - range;
	m_MaxY = startY + range;
	//Position the bullet ready to be drawn
	m_BulletShape.setPosition(m_Position);
}
void Bullet::stop() {
	m_InFlight = false;
}
bool Bullet::isInFlight() {
	return m_InFlight;
}
FloatRect Bullet::getPosition() {
	return m_BulletShape.getGlobalBounds();
}
RectangleShape Bullet::getShape() {
	return m_BulletShape;
}
void Bullet::update(float elapsedTime) {
	m_Position.x += m_BulletDistanceX * elapsedTime;
	m_Position.y += m_BulletDistanceY * elapsedTime;
	//Move the buller
	m_BulletShape.setPosition(m_Position);
	if (m_Position.x < m_MinX || m_Position.x > m_MaxX || m_Position.y < m_MinY || m_Position.y > m_MaxY)
	{
		m_InFlight = false;
	}
}