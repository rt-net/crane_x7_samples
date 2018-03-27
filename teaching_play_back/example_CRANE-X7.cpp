// *********    Example teaching play back    *********
//
//

#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "dynamixel_sdk.h"                                  // Dynamixel SDK library�̃C���N���[�h

// Control table address						            // Dynamixel �R���g���[���e�[�u���Q�Ƃ̃A�h���X�l
#define ADDR_PRO_TORQUE_ENABLE          64
#define ADDR_PRO_GOAL_POSITION          116
#define ADDR_PRO_PRESENT_POSITION       132
#define ADDR_PRO_PROFILE_VELOCITY		112

// Data Byte Length											// Dynamixel �A�h���X���̃f�[�^��
#define LEN_PRO_LED_RED                 1
#define LEN_PRO_GOAL_POSITION           4
#define LEN_PRO_PRESENT_POSITION        4
#define LEN_PRO_PROFILE_VELOCITY		4

// Protocol version
#define PROTOCOL_VERSION                2.0                 // Dynamixel�ʐM�v���g�R��

// Default setting
#define BAUDRATE                        3000000				// �T�[�{�̒ʐM���[�g
#define DEVICENAME                      "/dev/ttyUSB0"      // PC�ɐڑ����Ă���|�[�g
// ex) Windows: "COM1"   Linux: "/dev/ttyUSB0" Mac: "/dev/tty.usbserial-*"

#define TORQUE_ENABLE                   1                   // �g���NON�p�̃t���O
#define TORQUE_DISABLE                  0                   // �g���NOFF�p�̃t���O

#define DXL_MOVING_STATUS_THRESHOLD     20                  // Dynamixel moving status threshold
#define DXL_CENTER_POSITION_VALUE		2048				// Offset(�o�͎��̒��S�p�x)�@(value)
#define DXL_PROFILE_VELOCITY			30					// �T�[�{���[�^�̓��쑬�x�@(rpm)

#define JOINT_NUM						8					// �T�[�{���[�^�̌�(���R�x��7)

static int		ID[JOINT_NUM]			=	{	2,		3,		4,		5,		6,		7,		8,		9};			// �T�[�{���[�^��ID
static double	JOINT_MIN[JOINT_NUM]	=	{	262,	1024,	262,	228,	262,	1024,	148,	1991};		// �T�[�{���[�^�̍ŏ�����p(value)
static double	JOINT_MAX[JOINT_NUM]	=	{	3834,	3072,	3834,	2048,	3834,	3072,	3948,	3072};		// �T�[�{���[�^�̍ő哮��p(value)
static double	goal_pose[JOINT_NUM]	=	{	0,		0,		0,		0,		0,		0,		0,		0};			// Move_goal_position�֐��̈���(deg)

double deg2value( double deg ){ return (deg + 180) * 4096 / 360; }		// deg��value�ɕϊ�(�T�[�{�o�͎��̊��0)(value)
double value2deg( double value){ return value * 360 / 4096 - 180; }		// value��deg�ɕϊ�(�T�[�{�o�͎��̊��0)(deg)

/**
 * @fn		int getch()
 * @brief	�L�[�{�[�h���͗p�̊֐�(Dynamixel SDK sample)
 * @return	getchar() �L�[����
*/
int getch()		
{
	struct termios oldt, newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

/**
 * @brief CRANE-x7 ����p�N���X
 */
class CR7 {
	public:
	// Initialize PortHandler instance
	// Set the port path
	// Get methods and members of PortHandlerLinux or PortHandlerWindows
	// �g�p����|�[�g�̊e��ݒ�(Dynamixel SDK)
	dynamixel::PortHandler *portHandler;

	// Initialize PacketHandler instance
	// Set the protocol version
	// Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
	// �T�[�{���[�^�ƒʐM���邽�߂̃p�P�b�g�̐ݒ�(Dynamixel SDK)
	dynamixel::PacketHandler *packetHandler;

	// Initialize GroupBulkWrite instance
	// �����T�[�{�̏������ݗp�֐��̌Ăяo��(Dynamixel SDK)
	dynamixel::GroupBulkWrite *groupBulkWrite;

	// Initialize GroupBulkRead instance
	// �����T�[�{�̓ǂݍ��ݗp�֐��̌Ăяo��(Dynamixel SDK)
	dynamixel::GroupBulkRead *groupBulkRead;

	CR7();		//�R���X�g���N�^

	//�e��G���[�o�͗p�ϐ�
	int dxl_comm_result;             // Communication result
	bool dxl_addparam_result;               // addParam result
	bool dxl_getdata_result;                // GetParam result
	uint8_t dxl_error;                          // Dynamixel error

	uint8_t param_goal_position[4];		//�ʐM�p�P�b�g�p�ɕϊ�����goal_position�̕ϐ�
	uint8_t param_value[4];				//�ʐM�p�P�b�g�p�ɕϊ�����value�̕ϐ�
	
	int32_t dxl_present_position;		//�T�[�{�̌��݈ʒu�擾�p�̕ϐ�
	
	//�@play back�p�e�L�X�g�f�[�^�̒�`
	FILE *fp;
	const char *fname = "date.txt";

	bool Open_port();							//�ʐM�|�[�g���J��
	bool Set_port_baudrate();					//�ʐM���[�g�̐ݒ�
	void Enable_Dynamixel_Torque();				//�S�T�[�{�̃g���N��ON
	void Disable_Dynamixel_Torque();			//�S�T�[�{�̃g���N��OFF
	void Move_Goal_Position(double *goal_pose);	//�ݒ肳��Ă���goal position�Ɉړ�
	void Move_Offset_Position();				//�����ʒu(�o�͎����S�p)�ւ̈ړ�
	bool Teaching_Play_Frame();					//play back�p�̃e�L�X�g�f�[�^���쐬
	bool Play_Back();							//�e�L�X�g�f�[�^�����ɓ���
	void Close_port();							//�ʐM�|�[�g�����
};

/**
 * @brief �R���X�g���N�^
 */
CR7::CR7(){
	portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
	packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);
	groupBulkWrite = new dynamixel::GroupBulkWrite( portHandler, packetHandler);
	groupBulkRead = new dynamixel::GroupBulkRead( portHandler, packetHandler);

	dxl_comm_result = COMM_TX_FAIL;             // Communication result
	dxl_addparam_result = false;               // addParam result
	dxl_getdata_result = false;                // GetParam result
	dxl_error = 0;                          // Dynamixel error

	dxl_present_position = 0;
}

/**
 * @fn		bool Open_port()
 * @brief	�ݒ肵�� DEVICENAME�̃|�[�g���J��
 * @return	bool 1:�����@0:���s
 */
bool CR7::Open_port(){
	//Open port
	if (portHandler->openPort())
	{
		printf("Succeeded to open the port!\n");
		return 1;
	}
	else
	{
		printf("Failed to open the port!\n");
		printf("Press any key to terminate...\n");
		getch();
		return 0;
	}
}

/**
 * @fn		bool Set_port_baudrate()
 * @brief	�ݒ肵�� BAUDRATE�ŒʐM�̐ݒ������
 * @return	bool 1:���� 0:���s
 */
bool CR7::Set_port_baudrate(){
	// Set port baudrate
	if (portHandler->setBaudRate(BAUDRATE))
	{
		printf("Succeeded to change the baudrate!\n");
		return 1;
	}
	else
	{
		printf("Failed to change the baudrate!\n");
		printf("Press any key to terminate...\n");
		getch();
		return 0;
	}
}

/**
 * @fn		void Enable_Dynamixel_Torque()
 * @brief	�S�T�[�{�̃g���N��ON�ɂ���
 *			�S�T�[�{�̉�]���x��DXL_PROFILE_VELOCITY�ɐݒ�
 */
void CR7::Enable_Dynamixel_Torque(){
	// Enable Dynamixel Torque
	for(int i=0;i<JOINT_NUM;i++){
		dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, ID[i], ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);			//�Y��ID�̃T�[�{�̃g���N�Ǘ��̃A�h���X��ON����������
		param_value[0] = DXL_LOBYTE(DXL_LOWORD(DXL_PROFILE_VELOCITY));																	//�ݒ肵����]���x��ʐM�p�P�b�g�p�Ƀf�[�^�𕪂���
		param_value[1] = DXL_HIBYTE(DXL_LOWORD(DXL_PROFILE_VELOCITY));
		param_value[2] = DXL_LOBYTE(DXL_HIWORD(DXL_PROFILE_VELOCITY));
		param_value[3] = DXL_HIBYTE(DXL_HIWORD(DXL_PROFILE_VELOCITY));
		dxl_addparam_result = groupBulkWrite->addParam(ID[i], ADDR_PRO_PROFILE_VELOCITY, LEN_PRO_PROFILE_VELOCITY, param_value);		//�������ݗp�̃p�P�b�g�ɍ쐬�����f�[�^��ǉ�
		printf("[ ID : %d : ", ID[i]);																									//�e�T�[�{�����M�����p�P�b�g�ǂ���ɓ����Ă��邩�m�F
		if (dxl_comm_result != COMM_SUCCESS) printf(" result : %s", packetHandler->getTxRxResult(dxl_comm_result));						//�������R�}���h�����M����Ă��邩�m�F
		else if (dxl_error != 0) printf(" error : %s", packetHandler->getRxPacketError(dxl_error));										//�G���[�����������ꍇ�̃R�����g
		else printf(" successfully connected ");																						//����ɃT�[�{���g���NON
		printf(" ]\n");
	}

	// Bulkwrite goal position 
	dxl_comm_result = groupBulkWrite->txPacket();
	if (dxl_comm_result != COMM_SUCCESS) printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));

	// Clear bulkwrite parameter storage
	groupBulkWrite->clearParam();
}

/**
 * @fn		void Disable_Dynamixel_Torque()
 * @brief	�S�T�[�{�̃g���N��OFF�ɂ���
 */
void CR7::Disable_Dynamixel_Torque(){
	// Disable Dynamixel Torque
	for(int i=0;i<JOINT_NUM;i++)
		dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, ID[i], ADDR_PRO_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);		//�Y��ID�̃T�[�{�̃g���N�Ǘ��̃A�h���X��OFF����������

	// Bulkwrite goal position 
	dxl_comm_result = groupBulkWrite->txPacket();
	if (dxl_comm_result != COMM_SUCCESS) printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));

	// Clear bulkwrite parameter storage
	groupBulkWrite->clearParam();
}

/**
 * @fn		void Move_Goal_Position()
 * @brief	�ݒ肵�Ă���Goal Position�ֈړ�
 * @param	goal_pose[8](static double goal_pose[8]) �T�[�{�̌����̃f�[�^(deg)
 */
void CR7::Move_Goal_Position( double *goal_pose){
	//Move target goal position
	for(int i=0;i<JOINT_NUM;i++){
		printf("[ ID[%d] : %lf ]", ID[i], goal_pose[i]);																				//�w�肵���T�[�{�ƃf�[�^�̊m�F
		if((JOINT_MIN[i] > deg2value(goal_pose[i])) || (JOINT_MAX[i] < deg2value(goal_pose[i]))){										//����p�x�O�̊p�x�����͂��ꂽ�ꍇ
			printf("over range!\n");																										//���̓f�[�^��deg�Ȃ̂�value�ɕϊ�(���S��0�ɂȂ�)
			break;
		}

		param_goal_position[0] = DXL_LOBYTE(DXL_LOWORD(deg2value(goal_pose[i])));														//�ʐM�p�Ƀf�[�^�𕪂���
		param_goal_position[1] = DXL_HIBYTE(DXL_LOWORD(deg2value(goal_pose[i])));
		param_goal_position[2] = DXL_LOBYTE(DXL_HIWORD(deg2value(goal_pose[i])));
		param_goal_position[3] = DXL_HIBYTE(DXL_HIWORD(deg2value(goal_pose[i])));

		dxl_addparam_result = groupBulkWrite->addParam(ID[i], ADDR_PRO_GOAL_POSITION, LEN_PRO_GOAL_POSITION, param_goal_position);		//�������ݗp�̃p�P�b�g�ɒǉ�
		if(dxl_addparam_result != true) printf("goal pose error!\n");

	}printf("\n");
	
	// Bulkwrite goal position 
	dxl_comm_result = groupBulkWrite->txPacket();
	if (dxl_comm_result != COMM_SUCCESS) printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));

	// Clear bulkwrite parameter storage
	groupBulkWrite->clearParam();
}

/**
 * @fn		void Move_Offset_Position()
 * @brief	�T�[�{�̏����ʒu(�o�͎����S�p�x)�ֈړ�
 */
void CR7::Move_Offset_Position(){
	// Move offset position
	for(int i=0;i<JOINT_NUM;i++){ 
		param_goal_position[0] = DXL_LOBYTE(DXL_LOWORD(DXL_CENTER_POSITION_VALUE));														//�ʐM�p�Ƀf�[�^�𕪂���
		param_goal_position[1] = DXL_HIBYTE(DXL_LOWORD(DXL_CENTER_POSITION_VALUE));
		param_goal_position[2] = DXL_LOBYTE(DXL_HIWORD(DXL_CENTER_POSITION_VALUE));
		param_goal_position[3] = DXL_HIBYTE(DXL_HIWORD(DXL_CENTER_POSITION_VALUE));

		dxl_addparam_result = groupBulkWrite->addParam(ID[i], ADDR_PRO_GOAL_POSITION, LEN_PRO_GOAL_POSITION, param_goal_position);		//�������ݗp�̃p�P�b�g�ɒǉ�
	}
	if(dxl_addparam_result != true) printf("offset error!\n");

	// Bulkwrite goal position 
	dxl_comm_result = groupBulkWrite->txPacket();
	if (dxl_comm_result != COMM_SUCCESS) printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));

	// Clear bulkwrite parameter storage
	groupBulkWrite->clearParam();
}

/**
 * @fn		bool Teaching_Play_Frame()
 * @brief	Play back�p�̃e�L�X�g�f�[�^�̍쐬
 * @return	bool 1:���� -1:���s(�e�L�X�g�f�[�^���J���Ȃ�)
 */
bool CR7::Teaching_Play_Frame(){
	//Teaching play back motion frame
	fp = fopen( fname, "w");																											//���[�h:w(�������ݐ�p)�Ńt�@�C�����J��
	if(fp == NULL){
		printf("can not open\n");
		return -1;
	}
	while(1){																															//�f�[�^�擾�̊J�n
		printf("teaching play frame( type q end teaching )\n");
		char ch2 = getch();																												//�@"q"�@�ȊO�̃L�[�������ꂽ���̊p�x���e�L�X�g�f�[�^�֕ۑ�
		if(ch2 == 'q')break;
		for(int i=0;i<JOINT_NUM;i++){
			dxl_addparam_result = groupBulkRead->addParam(ID[i], ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);					//�ǂݍ��݂̃f�[�^��ݒ�(���݊p�x)
			//if( dxl_addparam_result != true) printf(" ID[%d] : groupBulkRead addParam failed\n", ID[i]);

			//Bulkread present position 
			dxl_comm_result = groupBulkRead->txRxPacket();																				//�ԐM�f�[�^�̓ǂݍ���
			if(dxl_comm_result != COMM_SUCCESS) printf(" discommect \n");

			//Check if groupbulkread data of Dynamixel is available																		//�ԐM�f�[�^�����p�ł��邩�m�F
			dxl_getdata_result = groupBulkRead->isAvailable(ID[i], ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);
			if(dxl_getdata_result != true) printf(" ID[%d] : groupBulkRead getdata failed\n", ID[i]);

			dxl_present_position = groupBulkRead->getData(ID[i], ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);					//�ԐM�f�[�^����w��̃f�[�^��ǂ�
			printf("[ ID[%d] : %lf ]", ID[i], value2deg(dxl_present_position));
			fprintf(fp, "%d,", dxl_present_position);
		}printf("\n");
		fprintf(fp,"\n");																												//�T�[�{������1�Ƃ��ĉ��s
	}
	fclose(fp);																															//�t�@�C�������
	return 1;
}

/**
 * @fn		bool Play_Back()
 * @brief	�e�L�X�g�f�[�^�̒l���Đ�
 * @return	bool 1:���� -1:���s(�e�L�X�g�f�[�^���J���Ȃ�)
 */
bool CR7::Play_Back(){
	double pose[8];
	//Play back date position
	fp = fopen( fname, "r");																											//���[�h:r(�ǂݍ��ݐ�p)�Ńt�@�C�����J��
	if(fp == NULL){
		printf("can not open\n");
		return -1;
	}
	while(	(fscanf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,", &pose[0],&pose[1],&pose[2],&pose[3],&pose[4],&pose[5],&pose[6],&pose[7])) != EOF){
		for(int i=0;i<JOINT_NUM;i++){																									//EOF�܂�1�s���f�[�^��ǂݍ���
			printf("[ ID[%d] : %lf ]", ID[i], value2deg(pose[i]));
			if((JOINT_MIN[i] > pose[i]) || (JOINT_MAX[i] < pose[i])){															//����p�x�O�̊p�x�����͂��ꂽ�ꍇ
				printf("over range!\n");
				break;
			}

			param_goal_position[0] = DXL_LOBYTE(DXL_LOWORD(pose[i]));																//�ʐM�p�f�[�^�ɕ�����
			param_goal_position[1] = DXL_HIBYTE(DXL_LOWORD(pose[i]));
			param_goal_position[2] = DXL_LOBYTE(DXL_HIWORD(pose[i]));
			param_goal_position[3] = DXL_HIBYTE(DXL_HIWORD(pose[i]));

			dxl_addparam_result = groupBulkWrite->addParam(ID[i], ADDR_PRO_GOAL_POSITION, LEN_PRO_GOAL_POSITION, param_goal_position);	//�������ݗp�p�P�b�g�Ƀf�[�^��ǉ�
			if(dxl_addparam_result != true) printf("goal pose error!\n");

			// Bulkwrite goal position 
			dxl_comm_result = groupBulkWrite->txPacket();
			if (dxl_comm_result != COMM_SUCCESS) printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));

			// Clear bulkwrite parameter storage
			groupBulkWrite->clearParam();
		}printf("\n");
		sleep(3);																														//�f�[�^���Đ����I���p��sleep���s��
	}
	fclose(fp);
	return 1;
}

/**
 * @fn		void Close_port()
 * @brief	�ʐM�|�[�g�����
 */
void CR7::Close_port(){
	// Close port
	portHandler->closePort();
	printf("port close and exit program\n");
}

/**
 * @fn		main()
 * @brief	main
 */
int main()
{
	CR7 cr;																																//�N���X�̐錾
	if(!cr.Open_port()) return 0;																										//COM�|�[�g���J��
	if(!cr.Set_port_baudrate()) return 0;																								//�|�[�g�̒ʐM���[�g��ݒ�

	while(1){
		printf("q:exit o:[SERVO ON] i:[SERVO OFF] p:[MOVE POSITION] s:[Offset Position] t:[Teaching] l:[Play back date]\n");
		char ch = getchar();							//�L�[���́@"q":�v���O�����I��
		switch(ch){
			case 'q':									//�v���O�����I��
				cr.Disable_Dynamixel_Torque();				//�g���NOFF
				return 0;
				break;
			case 'o':									//�T�[�{�g���NON
				cr.Enable_Dynamixel_Torque();
				break;
			case 'i':									//�T�[�{�g���NOFF
				cr.Disable_Dynamixel_Torque();
				break;
			case 'p':									//�w��̊p�x�Ɉړ�
				cr.Move_Goal_Position( goal_pose );
				break;
			case 's':									//�����p���Ɉړ�
				cr.Move_Offset_Position();
				break;
			case 't':									//�e�L�X�g�Ɋp�x��ۑ�
				cr.Disable_Dynamixel_Torque();				//�g���NOFF
				cr.Teaching_Play_Frame();
				break;
			case 'l':									//�e�L�X�g�f�[�^�̍Đ�
				printf("----ATTENTION!----\n");
				cr.Enable_Dynamixel_Torque();				//�g���NON
				sleep(3);									//�ˑR���삵�Ȃ��悤�ɃX���[�v
				cr.Move_Offset_Position();					//�����p���Ɉړ�
				cr.Play_Back();								//�f�[�^�̍Đ�
				cr.Move_Offset_Position();					//�����p���Ɉړ�
				printf("--FINISH--\n");
				break;
		}	
	}
	cr.Close_port();																													//COM�|�[�g�����
	return 0;
}

