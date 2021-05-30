#include "framework.h"
#include "Settings.h"
#include "EngineBase.h"
#include "Engine.h"
#include "Level.h"
#include "HUD.h"

double Engine::offset = 0;

Engine::Engine()
{
    // �������������� �������
    level = new Level();
    AddGameObject(level);

    // �������������� �����
    character = new Character();
    AddGameObject(character);

    // �������������� �����
    noEnemies = 3;
    enemies[0] = new Enemy(425, 350, 625, 0);
    AddGameObject(enemies[0]);
    enemies[1] = new Enemy(1475, 150, 1525, 7);
    AddGameObject(enemies[1]);
    enemies[2] = new Enemy(3275, 300, 3475, 11);
    AddGameObject(enemies[2]);

    // �������������� HUD
    hud = new HUD();
    AddGameObject(hud);

    leftPressed = false;
    rightPressed = false;
    spacePressed = false;
}

Engine::~Engine()
{
}

void Engine::KeyUp(WPARAM wParam)
{
    // ���������� ������ ����������
    if (wParam == VK_LEFT)
        leftPressed = false;
    if (wParam == VK_RIGHT)
        rightPressed = false;
    if (wParam == VK_SPACE)
        spacePressed = false;
}

void Engine::KeyDown(WPARAM wParam)
{
    // ������������� � ���������� ������� ������
    if (wParam == VK_LEFT)
        leftPressed = true;
    if (wParam == VK_RIGHT)
        rightPressed = true;
    if (wParam == VK_SPACE)
        spacePressed = true;
}

void Engine::Logic(double elapsedTime)
{
    EngineBase::Logic(elapsedTime);

        // ��� ���������� ����� ������. ������� ���� ���� ������� ������.
        // �� ��������� ������ ���� � �������� �����, ��������� ����� ��������� ������ � ����, � ��������.
        // ���������� ��� �������� ��� �������� � ����������������� �����������, ���������� �� �������� ���������� ��� �������

    if (!character->IsDead()) // ��� ������ ���� ���������� ������ � ��� ������, ���� �������� �� �����
    {
        // �������� ������ ���������� ����
        character->GoesLeft = leftPressed;
        character->GoesRight = rightPressed;

        // ���� �� �������� ������, ��������� �� � HUD
        hud->AddCoins(level->PickUpCollectibles(character));

        // If we collide with the tiles of the level
        CollisionDistances cd = level->CharacterCollides(character);
        if (cd.bottom > 0)
        {
            // if it's a bottom collision, we stop the character from falling
            character->StopFalling(cd.bottom);
            if (spacePressed)
            { // if the user pressed the jump key, the character jumps
                character->Jump(true);
                spacePressed = false;
            }
        }
        if (cd.left > 0)
        {
            // ���� ��������� ������������ �����
            character->StopMovingLeft(cd.left);
        }
        if (cd.right > 0)
        {
            // ���� ��������� ������������ ������
            character->StopMovingRight(cd.right);
        }
        if (cd.top > 0)
        {
            // ���� ��������� ������������ ������
            character->BounceTop();
        }

        for (int i = 0; i < noEnemies; i++)
        { // �������� ������������ � ������
            CollisionDistances cd = enemies[i]->CharacterCollides(character);
            if (cd.bottom > 0)
            {
                // ���� �� ���������� �� �����, �� ������������
                Enemy* enemy = enemies[i];
                for (int k = i; k < noEnemies - 1; k++)
                {
                    enemies[k] = enemies[k + 1];
                }
                enemies[noEnemies - 1] = NULL;
                RemoveGameObject(enemy);
                delete enemy;
                noEnemies--;
                character->Jump(false);
            }
            else if (cd.left > 0 || cd.right > 0 || cd.top > 0)
            {
                // ���� �� ������������ � ������ � ����� ������ �������, �������� �������
                character->Die();
                hud->RemoveLife();
            }
        }

        if (character->GetPosition().y > RESOLUTION_Y)
        {
            // ���� �������� �������� �� ����� ������, �� �������
            character->Die();
            hud->RemoveLife();
        }

        if (level->LevelExit(character))
        {
            // ���� �������� ������� �� �������� �����, �� �� �����������
            hud->FinishedLevel();
        }

        // �������� �������� ������� � ����������� �� ��������� ���������, �� ������������ ��� �� �������
        Engine::offset = character->GetPosition().x - RESOLUTION_X / 2;
        if (Engine::offset < 0)
            Engine::offset = 0;
        if (Engine::offset > LEVEL_WIDTH* TILE_WIDTH - RESOLUTION_X)
            Engine::offset = LEVEL_WIDTH * TILE_WIDTH - RESOLUTION_X;
    }
    else
    {
        // ���� �������� ����� (���������) � �� �������� � ������� ����� ������, �� ���������� ����� (���� ��� ��� ����)
        if (character->GetPosition().y < 0 && hud->HasLives())
        {
            character->Reset();
        }
    }
}