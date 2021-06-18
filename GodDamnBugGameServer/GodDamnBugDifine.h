#pragma once

#define DEGREE_TO_RADIAN(degree) M_PI*(degree/180.0f)
#define RADIAN_TO_DEGREE(radian) (FLOAT)(radian*(180.0f/M_PI))


// X ��ǥ�� Ÿ�� ��ǥ ����
#define X_TILE(x) x*2

// Y ��ǥ�� Ÿ�� ��ǥ ����
#define Y_TILE(y) y*2

// X Ÿ�� ��ǥ�� X �� ��ǥ ���
#define X_BLOCK(x) (FLOAT)x/2.0f

// Y Ÿ�� ��ǥ�� Y �� ��ǥ ���
#define Y_BLOCK(y) (FLOAT)y/2.0f

constexpr DWORD X_RANGE = 200;

constexpr DWORD Y_RANGE = 100;

constexpr DWORD X_TILE_RANGE = 400;

constexpr DWORD Y_TILE_RANGE = 200;

// ������ X ����, �� X ���̰� �� ���� ���� ���� ������ �پ���.
constexpr DWORD X_SECTOR_LENGTH = 10;

// ������ Y ����
constexpr DWORD Y_SECTOR_LENGTH = 10;

// X ������ �� ����
constexpr DWORD X_SECTOR_RANGE = X_TILE_RANGE / X_SECTOR_LENGTH;

// Y ������ �� ����
constexpr DWORD Y_SECTOR_RANGE = Y_TILE_RANGE / Y_SECTOR_LENGTH;


constexpr INT PLAYER_ATTACK1_RANGE = 2;
constexpr INT PLAYER_ATTACK2_RANGE = 2;
constexpr INT MONSTER_ATTACK_RANGE = 2;

// Attack2 Ÿ�� ������ ������Ʈ ����
constexpr INT ATTACK2_VICTIM_COUNT = 3;

// x Ÿ�� ������ ���� X ��ǥ ����
#define X_SECTOR(X) X/X_SECTOR_LENGTH

// y Ÿ�� ������ ���� Y ��ǥ ����
#define Y_SECTOR(Y) Y/Y_SECTOR_LENGTH


constexpr INT CRISTAL_PICK_RANGE = 4;

constexpr INT X_MONSTER_RESPAWN_CENTER_1 = 26;
constexpr INT Y_MONSTER_RESPAWN_CENTER_1 = 169;
constexpr INT MONSTER_RESPAWN_RANGE_1 = 21;

constexpr INT X_MONSTER_RESPAWN_CENTER_2 = 95;
constexpr INT Y_MONSTER_RESPAWN_CENTER_2 = 169;
constexpr INT MONSTER_RESPAWN_RANGE_2 = 24;

constexpr INT X_MONSTER_RESPAWN_CENTER_3 = 163;
constexpr INT Y_MONSTER_RESPAWN_CENTER_3 = 152;
constexpr INT MONSTER_RESPAWN_RANGE_3 = 17;

constexpr INT X_MONSTER_RESPAWN_CENTER_4 = 171;
constexpr INT Y_MONSTER_RESPAWN_CENTER_4 = 33;
constexpr INT MONSTER_RESPAWN_RANGE_4 = 25;

constexpr INT X_MONSTER_RESPAWN_CENTER_5 = 34;
constexpr INT Y_MONSTER_RESPAWN_CENTER_5 = 61;
constexpr INT MONSTER_RESPAWN_RANGE_5 = 17;

constexpr INT X_MONSTER_RESPAWN_CENTER_6 = 219;
constexpr INT Y_MONSTER_RESPAWN_CENTER_6 = 179;
constexpr INT MONSTER_RESPAWN_RANGE_6 = 14;

constexpr INT X_MONSTER_RESPAWN_CENTER_7 = 317;
constexpr INT Y_MONSTER_RESPAWN_CENTER_7 = 114;
constexpr INT MONSTER_RESPAWN_RANGE_7 = 79;

// Ÿ�� ���� ���Ͱ� �� ���� �ִ� �̵��� �� �ִ� �Ÿ�
constexpr INT MONSTER_TILE_MOVE_RANGE = 2;


// ���Ͱ� ������ �� �ִ� ���ǵ�, 500ms ���� �����̰� ������ 500 ����
constexpr INT MONSTER_SPEED = 5000;

constexpr INT MONSTER_ATTACK_SPEED = 5000;

// �������� ���� ����
constexpr INT AREA1_MONSTER_COUNT = 15;
constexpr INT AREA2_MONSTER_COUNT = 15;
constexpr INT AREA3_MONSTER_COUNT = 15;
constexpr INT AREA4_MONSTER_COUNT = 15;
constexpr INT AREA5_MONSTER_COUNT = 15;
constexpr INT AREA6_MONSTER_COUNT = 20;
constexpr INT AREA7_MONSTER_COUNT = 20;

// ���� ��Ȱ �ð�
constexpr INT MONSTER_REVIVAL = 7000;

// ���Ͱ� �÷��̾ ���󰡴� �ð�
constexpr INT MONSTER_RAGE_TIME = 30000;

constexpr INT IP_LENGTH = 30;

constexpr DWORD TOKEN_SIZE = 64;

enum class eMonsterActivityArea
{
	MonsterArea1 = 1,
	MonsterArea2,
	MonsterArea3,
	MonsterArea4,
	MonsterArea5,
	MonsterArea6,
	MonsterArea7,
};


enum class eCharacterType
{
	Golem = 1,
	Knight,
	Orc,
	Elf,
	Archer,
	LastType
};

enum class eMonsterType
{
	Monster1 = 1
};

enum class eCristalType
{
	Cristal1 = 1,
	Cristal2,
	Cristal3
};


namespace player
{
	constexpr DWORD PLAYER_STRING = 20;
}


