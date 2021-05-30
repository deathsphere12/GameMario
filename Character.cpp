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
	if (!dead) // Мы делаем всю эту логику только в том случае, если мы живи
	{
		// сохраняем предыдущую позицию
		prevPosition = position;

		// обновим позицию x в зависимости от скорости, но убедимся, что мы не выходим за пределы уровня
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
		if (GoesLeft) // если нажата левая кнопка, ускоримся влево
		{
			speed.x -= 1500 * elapsedTime;
			if (speed.x < -300)
				speed.x = -300;
		}
		if (GoesRight) // если нажата правая кнопка, ускоримся вправо
		{
			speed.x += 1500 * elapsedTime;
			if (speed.x > 300)
				speed.x = 300;
		}
		if (!GoesLeft && !GoesRight) // если ни одна клавиша не нажата, и мы находимся на земле, создадим перетаскивание, чтобы персонаж остановился
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

		// обновление позиции y в зависимости от скорости
		position.y += speed.y * elapsedTime;
		// создание гравитации
		speed.y += 1500 * elapsedTime;

		// установим вперед, чтобы использовать отрисовку персонажа в правильном направлении
		if (speed.x > 0)
			forward = true;
		if (speed.x < 0)
			forward = false;
	}
	else
	{
		// если персонаж умирает, то медленно сползает вниз
		position.y -= 200 * elapsedTime;
	}
}

void Character::StopFalling(double collisionSize)
{
	// этот метод вызывается, когда мы сталкиваемся с дном, и нам нужно остановить падение
	if (!dead)
	{
		speed.y = 0;
		position.y -= collisionSize;
	}
}

void Character::Jump(bool fullJump)
{
	// этот метод используется когда мы нажимаем пробел
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
	// этот метод вызывается, когда мы сталкиваемся с верхом, и нам нужно отскочить назад
	if (!dead)
	{
		speed.y = abs(speed.y);
	}
}

void Character::StopMovingLeft(double collisionSize)
{
	// этот метод вызывается, когда мы сталкиваемся слева
	if (!dead)
	{
		speed.x = 0;
		position.x += collisionSize;
	}
}

void Character::StopMovingRight(double collisionSize)
{
	// этот метод вызывается, когда мы сталкиваемся справа
	if (!dead)
	{
		speed.x = 0;
		position.x -= collisionSize;
	}
}

void Character::Die()
{
	// смерть персонажа
	dead = true;
}

bool Character::IsDead()
{
	// возвращает значение, если персонаж мертв
	return dead;
}

void Character::Reset()
{
	// возраждает персонажа на начале уровня
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
	if (tileSetImg == NULL) // загружает изображение персонажа, если оно еще не загружено
	{
		tileSetImg = engine->LoadImageW(L"characters.png");
	}

	D2D1_RECT_F rectangle1 = D2D1::RectF(
		position.x - Engine::offset - CHARACTER_WIDTH / 2, position.y - CHARACTER_TILE_HEIGHT,
		position.x - Engine::offset + CHARACTER_WIDTH / 2 - 1, position.y
	);

	// это делается для того, чтобы решить, какую плитку в тайлсете отрисовать
	int posX = forward ? 3 : 4;
	int posY = 0;

	if (position.y != prevPosition.y) // когда персонаж находится в середине прыжка
	{
		posX = forward ? 1 : 3;
		posY = 2;
	}
	else if (position.x != prevPosition.x) // когда персонаж падает на землю
	{
		int step = (int)position.x % 100 / 25;
		posY = 1;
		posX = (forward ? step : 7 - step);
	}

	if (dead) // когда персонаж умирает
	{
		posX = forward ? 4 : 5;
		posY = 3;
	}

	D2D1_RECT_F rectangle2 = D2D1::RectF(
		posX * CHARACTER_WIDTH, posY * CHARACTER_TILE_HEIGHT,
		(posX + 1) * CHARACTER_WIDTH - 1, (posY + 1) * CHARACTER_TILE_HEIGHT - 1
	);

	// вырисовывает анимацию смерти
	m_pRenderTarget->DrawBitmap(tileSetImg, rectangle1, dead ? 0.6f : 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rectangle2);
}