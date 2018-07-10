#include "flash.h"
#include "pstorage.h"
#include "nrf_error.h"
#include "app_error.h"
#include <stdint.h>
#include <string.h>

pstorage_handle_t 				m_data_num_handle;
__align(4)  uint8_t advertising_num[4]={0};	
bool flash_store_flag = 0,flash_load_flag = 0,flash_clear_flag = 0,flash_update_flag = 0;
bool flash_callback_store_flag=0,flash_callback_load_flag=0,flash_callback_clear_flag=0,flash_callback_update_flag=0;
extern __align(4)  uint8_t password_forever_num[4];

void flash_callback( pstorage_handle_t *p_handle,  uint8_t op_code,  uint32_t result, uint8_t *p_data,  uint32_t data_len )                                                
{
	switch(op_code)
	{
		case PSTORAGE_STORE_OP_CODE:
			flash_callback_store_flag=1;
			if (result == NRF_SUCCESS)
			{
				flash_store_flag = 1; //�� flash store ��ɺ���λ��־
				printf("\n flash store success \n");					
			}				
			else 												
			{
				flash_store_flag = 0;
				printf("\n flash store fail \n");	
			}
		break;
		case PSTORAGE_LOAD_OP_CODE:
			flash_callback_load_flag=1;
			if (result == NRF_SUCCESS)
			{
				flash_load_flag = 1; //�� flash load ��ɺ���λ��־	
				printf("\n flash load success \n");			
			}				
			else 												flash_load_flag = 0;
		break;
		case PSTORAGE_CLEAR_OP_CODE:
			flash_callback_clear_flag=1;
			if (result == NRF_SUCCESS) 
			{
				flash_clear_flag = 1; //�� flash clear ��ɺ���λ��־	
				printf("\n flash clear success \n");				
			}				
			else 												flash_clear_flag = 0; 
		break;
		case PSTORAGE_UPDATE_OP_CODE:
			flash_callback_update_flag=1;
			if (result == NRF_SUCCESS) 
			{
				flash_update_flag = 1; //�� flash update ��ɺ���λ��־
				printf("\n flash update success \n");				
			}				
			else
			{
				flash_update_flag = 0;
				printf("\n flash update fail \n");	
			}				
		break;
	}
}

void flash_init(void)
{
	uint32_t err_code;
	pstorage_module_param_t  	param;
	
/***** ��Ϊ	device_manager_init(erase_bonds);  �Ѿ�ִ����һ�� �������ﲻִ���� *****/		
//	err_code = pstorage_init();   //��ʼ��flash
//	APP_ERROR_CHECK(err_code);

	
	param.block_size   =  40;   	// �������40byte��С
	param.block_count  =  4;     	//10��block

	param.cb           =  flash_callback;  //�����ص�
	
	err_code =pstorage_register( &param, &m_data_num_handle );  //ע��flash,û������룬����Ķ������޷��ɹ���
	while(err_code != NRF_SUCCESS);
	APP_ERROR_CHECK(err_code);	
}

void receive_phone_num_update(uint8_t * update_num)
{
	uint32_t err_code;
	pstorage_handle_t       block_handle;
	
	err_code = pstorage_block_identifier_get(&m_data_num_handle, 0, &block_handle); //��ȡid �� �ֻ��� 0
	APP_ERROR_CHECK(err_code);
//	while(1)
//	{
		// Request to update 11 bytes to block at an offset of pass_num*4 bytes.
		err_code = pstorage_update(&block_handle,update_num,12,4);	
		APP_ERROR_CHECK(err_code);
//		while(flash_callback_update_flag != 1);									//�ȴ�����ص�����
//		flash_callback_update_flag=0;
//		if(flash_update_flag == 1) 
//		{
//			flash_update_flag = 0;
//			break;
//		}
//	}		
}

void receive_phone_num_read(uint8_t * read_num)
{
	uint32_t err_code;
	pstorage_handle_t       block_handle;
	
	err_code = pstorage_block_identifier_get(&m_data_num_handle, 0, &block_handle); //��ȡid �� ���� 2
	APP_ERROR_CHECK(err_code);
//	while(1)
//	{
		// Request to read 12 bytes from block at an offset of pass_num*4 bytes.	
		err_code=pstorage_load(read_num,&block_handle,12,4);
		APP_ERROR_CHECK(err_code);
//		while(flash_callback_load_flag != 1);									//�ȴ�����ص�����
//		flash_callback_load_flag=0;
//		if(flash_load_flag == 1) 
//		{
//			flash_load_flag = 0;
//			break;
//		}
//	}	
}

void receive_password_num_update(uint8_t * update_num)
{
	uint32_t err_code;
	pstorage_handle_t       block_handle;
	err_code = pstorage_block_identifier_get(&m_data_num_handle, 2, &block_handle); //��ȡid �� ���� 2
	APP_ERROR_CHECK(err_code);
	
//	err_code = pstorage_clear(&m_data_num_handle,40);	
//	APP_ERROR_CHECK(err_code);
	
	err_code = pstorage_update(&block_handle,update_num,8,4);
	
//	printf("block_handle=%d\n",block_handle.block_id);
	APP_ERROR_CHECK(err_code);
	while(flash_callback_update_flag != 1);									//�ȴ�����ص�����
	flash_callback_update_flag=0;	
}

void receive_password_num_read(uint8_t * read_num)
{
	uint32_t err_code;
	pstorage_handle_t       block_handle;
	
	err_code = pstorage_block_identifier_get(&m_data_num_handle, 2, &block_handle); //��ȡid �� ���� 2
	APP_ERROR_CHECK(err_code);
//	while(1)
//	{
		// Request to read 11 bytes from block at an offset of pass_num*4 bytes.	
		err_code=pstorage_load(read_num,&block_handle,8,4);
		APP_ERROR_CHECK(err_code);
//		while(flash_callback_load_flag != 1);									//�ȴ�����ص�����
//		flash_callback_load_flag=0;
//		if(flash_load_flag == 1) 
//		{
//			flash_load_flag = 0;
//			break;
//		}
//	}	
}

void time_begin_Lock_update(uint8_t * update_num)
{
	uint32_t err_code;		
	pstorage_handle_t      		block_handle;
	
// Request to get identifier for 3rd block. 
	err_code = pstorage_block_identifier_get(&m_data_num_handle,3, &block_handle);			//��ȡid �� ��ʼ����ʱ�� 3
	APP_ERROR_CHECK(err_code);

//	while(1)
//	{
		// Request to update 8 bytes to block at an offset of 4 bytes.
		err_code = pstorage_update(&block_handle,update_num,8,4);	
		APP_ERROR_CHECK(err_code);
//		while(flash_callback_update_flag != 1);									//�ȴ�����ص�����
//		flash_callback_update_flag=0;
//		if(flash_update_flag == 1) 
//		{
//			flash_update_flag = 0;
//			break;
//		}
//	}		
}

void time_begin_Lock_read(uint8_t * read_num)
{
	uint32_t err_code;	
	pstorage_handle_t       block_handle;
	
	err_code = pstorage_block_identifier_get(&m_data_num_handle, 3, &block_handle);		//��ȡid �� ��ʼ����ʱ�� 3	 
	APP_ERROR_CHECK(err_code);
//	while(1)
//	{
		// Request to read 8 bytes from block at an offset of 4 bytes.	
		err_code=pstorage_load(read_num,&block_handle,8,4);
		APP_ERROR_CHECK(err_code);
//		while(flash_callback_load_flag != 1);									//�ȴ�����ص�����
//		flash_callback_load_flag=0;
//		if(flash_load_flag == 1) 
//		{
//			flash_load_flag = 0;
//			break;
//		}
//	}	
}

