#include "Settings.h"
#include "CollisionDistances.h"
#include "Character.h"
#include "Level.h"
#include "Engine.h"

Level::Level()
{
	
	// По умолчанию ничего не отображается
	for (int i = 0; i < LEVEL_HEIGHT; i++)
	{
		for (int j = 0; j < LEVEL_WIDTH; j++)
		{
			levelMatrix[i][j] = -1;
			levelSolids[i][j] = false;
			levelCollectible[i][j] = false;
		}
	}

	// Загружаем карту из файла
	std::ifstream myfile("level.txt");

	if (myfile.is_open())
	{
		for (int i = 0; i < LEVEL_HEIGHT; i++)
		{
			for (int j = 0; j < LEVEL_WIDTH; j++)
			{
				myfile >> levelMatrix[i][j];
				levelSolids[i][j] = tileIsSolid(levelMatrix[i][j]);
				levelCollectible[i][j] = tileIsCollectible(levelMatrix[i][j]);
			}
		}
	}
}

void Level::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{

	// Загружаем изображение если нужно
	if (tileSetImg == NULL)
	{
		tileSetImg = engine->LoadImageW(L"tileset.png");
	}

	if (cloudsImg == NULL)
	{
		cloudsImg = engine->LoadImageW(L"clouds.png");
	}

	// Нарисуем облака
	double start_x = Engine::offset / 2.0;
	while (start_x > RESOLUTION_X)
	{
		start_x -= RESOLUTION_X;
	}
	D2D1_RECT_F cloudsRect1 = D2D1::RectF(
		0 - start_x, 0,
		RESOLUTION_X - start_x, RESOLUTION_Y
	);
	D2D1_RECT_F cloudsRect2 = D2D1::RectF(
		RESOLUTION_X - start_x, 0,
		2 * RESOLUTION_X - start_x, RESOLUTION_Y
	);
	m_pRenderTarget->DrawBitmap(cloudsImg, cloudsRect1);
	m_pRenderTarget->DrawBitmap(cloudsImg, cloudsRect2);

	// Нарисуем плитки уровня
	for (int i = 0; i < LEVEL_HEIGHT; i++)
	{
		for (int j = 0; j < LEVEL_WIDTH; j++)
		{
			if ((j + 1) * TILE_WIDTH >= Engine::offset && j * TILE_WIDTH < Engine::offset + RESOLUTION_X)
			{
				if (levelMatrix[i][j] != -1) 
				{
					D2D1_RECT_F rectangle1 = D2D1::RectF(
						j * TILE_WIDTH - Engine::offset, i * TILE_WIDTH,
						(j + 1) * TILE_WIDTH - Engine::offset + 1, (i + 1) * TILE_WIDTH
					);

					int posY = levelMatrix[i][j] / 7;
					int posX = levelMatrix[i][j] % 7;

					D2D1_RECT_F rectangle2 = D2D1::RectF(
						posX * TILE_WIDTH, posY * TILE_WIDTH,
						posX * TILE_WIDTH + TILE_WIDTH - 1, posY * TILE_WIDTH + TILE_WIDTH - 1
					);

					m_pRenderTarget->DrawBitmap(tileSetImg, rectangle1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rectangle2);
				}
			}
		}
	}
}

CollisionDistances Level::CharacterCollides(Character* character)
{
	// этот метод возвращает инфо о столкновении между персонажем и плитками
	int noCollisions = 0;
	CollisionDistances collisions[9];

	CollisionDistances cummulatedCollision;
	cummulatedCollision.top = 0;
	cummulatedCollision.bottom = 0;
	cummulatedCollision.left = 0;
	cummulatedCollision.right = 0;

	double charTop = character->GetPosition().y - CHARACTER_HEIGHT;
	double charBottom = character->GetPosition().y;
	double charLeft = character->GetPosition().x - CHARACTER_WIDTH / 2 + 3;
	double charRight = character->GetPosition().x + CHARACTER_WIDTH / 2 - 4;

	// проверка только тех плиткок, рядом с которыми находится персонаж
	int startX = (int)(charLeft / TILE_WIDTH);
	if (startX < 0)
		startX = 0;
	int startY = (int)(charTop / TILE_WIDTH);
	if (startY < 0)
		startY = 0;
	int endX = (int)(charRight / TILE_WIDTH);
	if (endX > LEVEL_WIDTH - 1)
		endX = LEVEL_WIDTH - 1;
	int endY = (int)(charBottom / TILE_WIDTH);
	if (endY > LEVEL_HEIGHT - 1)
		endY = LEVEL_HEIGHT - 1;

	for (int i = startY; i <= endY; i++)
	{
		for (int j = startX; j <= endX; j++)
		{
			if (levelSolids[i][j])
			{
				double tileTop = i * TILE_WIDTH;
				double tileBottom = (i + 1) * TILE_WIDTH - 1;
				double tileLeft = j * TILE_WIDTH;
				double tileRight = (j + 1) * TILE_WIDTH - 1;

				// если это столкновение
				if (charTop < tileBottom && charBottom > tileTop && charRight > tileLeft && charLeft < tileRight)
				{
					// добавляем эти столкновения в список
					collisions[noCollisions].top = abs(charTop - tileBottom);
					collisions[noCollisions].bottom = abs(charBottom - tileTop);
					collisions[noCollisions].left = abs(charLeft - tileRight);
					collisions[noCollisions].right = abs(charRight - tileLeft);

					collisions[noCollisions].keepSmallest();
					
					// добавляем сведения о столкновении к проишедшему столкновению
					cummulatedCollision.top += collisions[noCollisions].top;
					cummulatedCollision.bottom += collisions[noCollisions].bottom;
					cummulatedCollision.left += collisions[noCollisions].left;
					cummulatedCollision.right += collisions[noCollisions].right;

					noCollisions++;
				}
			}
		}
	}

	// какая бы сторона ни столкнулась больше всего, эта сторона принимается во внимание
	cummulatedCollision.keepLargest();
	if (cummulatedCollision.top != 0)
	{
		cummulatedCollision.top = 0;
		for (int i = 0; i < noCollisions; i++)
		{
			cummulatedCollision.top = fmax(cummulatedCollision.top, collisions[i].top);
		}
	}
	if (cummulatedCollision.bottom != 0)
	{
		cummulatedCollision.bottom = 0;
		for (int i = 0; i < noCollisions; i++)
		{
			cummulatedCollision.bottom = fmax(cummulatedCollision.bottom, collisions[i].bottom);
		}
	}
	if (cummulatedCollision.left != 0)
	{
		cummulatedCollision.left = 0;
		for (int i = 0; i < noCollisions; i++)
		{
			cummulatedCollision.left = fmax(cummulatedCollision.left, collisions[i].left);
		}
	}
	if (cummulatedCollision.right != 0)
	{
		cummulatedCollision.right = 0;
		for (int i = 0; i < noCollisions; i++)
		{
			cummulatedCollision.right = fmax(cummulatedCollision.right, collisions[i].right);
		}
	}

	return cummulatedCollision;
}

int Level::PickUpCollectibles(Character* character)
{
	// этот метод проверяет наличие столкновений с монетами и выписывает номер
	int noCollisions = 0;
	
	double charTop = character->GetPosition().y - CHARACTER_HEIGHT;
	double charBottom = character->GetPosition().y;
	double charLeft = character->GetPosition().x - CHARACTER_WIDTH / 2 + 3;
	double charRight = character->GetPosition().x + CHARACTER_WIDTH / 2 - 4;

	int startX = (int)(charLeft / TILE_WIDTH);
	if (startX < 0)
		startX = 0;
	int startY = (int)(charTop / TILE_WIDTH);
	if (startY < 0)
		startY = 0;
	int endX = (int)(charRight / TILE_WIDTH);
	if (endX > LEVEL_WIDTH - 1)
		endX = LEVEL_WIDTH - 1;
	int endY = (int)(charBottom / TILE_WIDTH);
	if (endY > LEVEL_HEIGHT - 1)
		endY = LEVEL_HEIGHT - 1;

	for (int i = startY; i <= endY; i++)
	{
		for (int j = startX; j <= endX; j++)
		{
			if (levelCollectible[i][j])
			{
				double tileTop = i * TILE_WIDTH;
				double tileBottom = (i + 1) * TILE_WIDTH - 1;
				double tileLeft = j * TILE_WIDTH;
				double tileRight = (j + 1) * TILE_WIDTH - 1;

				// если это столкновение
				if (charTop < tileBottom && charBottom > tileTop && charRight > tileLeft && charLeft < tileRight)
				{
					noCollisions++;
					levelMatrix[i][j] = -1; // убираем монету, при сбрасывании положения плитки
					levelCollectible[i][j] = false;
				}
			}
		}
	}

	return noCollisions;
}

bool Level::LevelExit(Character* character)
{
	// возвращает значение true, если персонаж сталкивается с порталом выхода уровня
	int noCollisions = 0;

	double charTop = character->GetPosition().y - CHARACTER_HEIGHT;
	double charBottom = character->GetPosition().y;
	double charLeft = character->GetPosition().x - CHARACTER_WIDTH / 2 + 3;
	double charRight = character->GetPosition().x + CHARACTER_WIDTH / 2 - 4;

	int startX = (int)(charLeft / TILE_WIDTH);
	if (startX < 0)
		startX = 0;
	int startY = (int)(charTop / TILE_WIDTH);
	if (startY < 0)
		startY = 0;
	int endX = (int)(charRight / TILE_WIDTH);
	if (endX > LEVEL_WIDTH - 1)
		endX = LEVEL_WIDTH - 1;
	int endY = (int)(charBottom / TILE_WIDTH);
	if (endY > LEVEL_HEIGHT - 1)
		endY = LEVEL_HEIGHT - 1;

	for (int i = startY; i <= endY; i++)
	{
		for (int j = startX; j <= endX; j++)
		{
			if (levelMatrix[i][j] == 11)
			{
				double tileTop = i * TILE_WIDTH;
				double tileBottom = (i + 1) * TILE_WIDTH - 1;
				double tileLeft = j * TILE_WIDTH;
				double tileRight = (j + 1) * TILE_WIDTH - 1;

				// это столкновение
				if (charTop < tileBottom && charBottom > tileTop&& charRight > tileLeft&& charLeft < tileRight)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool Level::tileIsSolid(int tileNo)
{
	for (int i = 0; i < 34; i++)
	{
		if (solids[i] == tileNo)
		{
			return true;
		}
	}
	return false;
}

bool Level::tileIsCollectible(int tileNo)
{
	for (int i = 0; i < 8; i++)
	{
		if (collectibles[i] == tileNo)
		{
			return true;
		}
	}
	return false;
}