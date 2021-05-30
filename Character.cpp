#include "Settings.h"
#include "Character.h"
#include "Level.h"
#include "Engine.h"

Character::Character()
{
	position.x = 150;
	position.y = RESOLUTION_Y - 250;

	speed.x = 0;
	speed.y = 0;

	GoesLeft = false;
	GoesRight = false;

	forward = true;
	dead = false;
}

void Character::Logic(double elapsedTime)
{
	if (!dead) // �� ������ ��� ��� ������ ������ � ��� ������, ���� �� ����
	{
		// ��������� ���������� �������
		prevPosition = position;

		// ������� ������� x � ����������� �� ��������, �� ��������, ��� �� �� ������� �� ������� ������
		position.x += speed.x * elapsedTime;
		if (position.x < CHARACTER_WIDTH / 2)
		{
			position.x = CHARACTER_WIDTH / 2;
			speed.x = 0;
		}
		if (position.x > LEVEL_WIDTH* TILE_WIDTH - CHARACTER_WIDTH / 2)
		{
			position.x = LEVEL_WIDTH * TILE_WIDTH - CHARACTER_WIDTH / 2;
			speed.x = 0;
		}
		if (GoesLeft) // ���� ������ ����� ������, ��������� �����
		{
			speed.x -= 1500 * elapsedTime;
			if (speed.x < -300)
				speed.x = -300;
		}
		if (GoesRight) // ���� ������ ������ ������, ��������� ������
		{
			speed.x += 1500 * elapsedTime;
			if (speed.x > 300)
				speed.x = 300;
		}
		if (!GoesLeft && !GoesRight) // ���� �� ���� ������� �� ������, � �� ��������� �� �����, �������� ��������������, ����� �������� �����������
		{
			if (speed.y == 0)
			{
				if (speed.x > 0)
				{
					speed.x -= 1500 * elapsedTime;
					if (speed.x < 10)
						speed.x = 0;
				}
				if (speed.x < 0)
				{
					speed.x += 1500 * elapsedTime;
					if (speed.x > -10)
						speed.x = 0;
				}
			}
		}

		// ���������� ������� y � ����������� �� ��������
		position.y += speed.y * elapsedTime;
		// �������� ����������
		speed.y += 1500 * elapsedTime;

		// ��������� ������, ����� ������������ ��������� ��������� � ���������� �����������
		if (speed.x > 0)
			forward = true;
		if (speed.x < 0)
			forward = false;
	}
	else
	{
		// ���� �������� �������, �� �������� �������� ����
		position.y -= 200 * elapsedTime;
	}
}

void Character::StopFalling(double collisionSize)
{
	// ���� ����� ����������, ����� �� ������������ � ����, � ��� ����� ���������� �������
	if (!dead)
	{
		speed.y = 0;
		position.y -= collisionSize;
	}
}

void Character::Jump(bool fullJump)
{
	// ���� ����� ������������ ����� �� �������� ������
	if (!dead)
	{
		if (fullJump)
			speed.y = -700;
		else
			speed.y = -400;
	}
}

void Character::BounceTop()
{
	// ���� ����� ����������, ����� �� ������������ � ������, � ��� ����� ��������� �����
	if (!dead)
	{
		speed.y = abs(speed.y);
	}
}

void Character::StopMovingLeft(double collisionSize)
{
	// ���� ����� ����������, ����� �� ������������ �����
	if (!dead)
	{
		speed.x = 0;
		position.x += collisionSize;
	}
}

void Character::StopMovingRight(double collisionSize)
{
	// ���� ����� ����������, ����� �� ������������ ������
	if (!dead)
	{
		speed.x = 0;
		position.x -= collisionSize;
	}
}

void Character::Die()
{
	// ������ ���������
	dead = true;
}

bool Character::IsDead()
{
	// ���������� ��������, ���� �������� �����
	return dead;
}

void Character::Reset()
{
	// ���������� ��������� �� ������ ������
	position.x = 150;
	position.y = RESOLUTION_Y - 250;

	speed.x = 0;
	speed.y = 0;

	GoesLeft = false;
	GoesRight = false;

	forward = true;
	dead = false;
}

void Character::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	if (tileSetImg == NULL) // ��������� ����������� ���������, ���� ��� ��� �� ���������
	{
		tileSetImg = engine->LoadImageW(L"characters.png");
	}

	D2D1_RECT_F rectangle1 = D2D1::RectF(
		position.x - Engine::offset - CHARACTER_WIDTH / 2, position.y - CHARACTER_TILE_HEIGHT,
		position.x - Engine::offset + CHARACTER_WIDTH / 2 - 1, position.y
	);

	// ��� �������� ��� ����, ����� ������, ����� ������ � �������� ����������
	int posX = forward ? 3 : 4;
	int posY = 0;

	if (position.y != prevPosition.y) // ����� �������� ��������� � �������� ������
	{
		posX = forward ? 1 : 3;
		posY = 2;
	}
	else if (position.x != prevPosition.x) // ����� �������� ������ �� �����
	{
		int step = (int)position.x % 100 / 25;
		posY = 1;
		posX = (forward ? step : 7 - step);
	}

	if (dead) // ����� �������� �������
	{
		posX = forward ? 4 : 5;
		posY = 3;
	}

	D2D1_RECT_F rectangle2 = D2D1::RectF(
		posX * CHARACTER_WIDTH, posY * CHARACTER_TILE_HEIGHT,
		(posX + 1) * CHARACTER_WIDTH - 1, (posY + 1) * CHARACTER_TILE_HEIGHT - 1
	);

	// ������������ �������� ������
	m_pRenderTarget->DrawBitmap(tileSetImg, rectangle1, dead ? 0.6f : 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rectangle2);
}