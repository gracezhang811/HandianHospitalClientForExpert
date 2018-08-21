#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <process.h>

#include "msp_cmn.h"
#include "msp_errors.h"
#include "speech_recognizer.h"

#include "myvoice.h"

#define	BUFFER_SIZE	4096
#define LOG_SIZE (260)

static struct speech_rec iat;
static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

char ** m_pResult = NULL;

void on_result(const char *result, char is_last)
{
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		//show_result(g_result, is_last);

		printf("Result: [ %s ]\n", g_result);

		*m_pResult = g_result;
	}
}
void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	printf("Start Listening...\n");
}
void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
	{
		printf("\nSpeaking done \n");
	}		
	else
	{
		printf("\nRecognizer error %d\n", reason);
	}
}

int InitVoice()
{
	int			ret = MSP_SUCCESS;
	int			upload_on = 1; //�Ƿ��ϴ��û��ʱ�
	int			mic_on = 1; //�Ƿ�ʼmicʶ��
	int			exit_on = 1; //�Ƿ����
	const char* login_params = "appid = 59969e54, work_dir = ."; // ��¼������appid��msc���,��������Ķ�
	int aud_src = 0;

	/*
	* sub:				����ҵ������
	* domain:			����
	* language:			����
	* accent:			����
	* sample_rate:		��Ƶ������
	* result_type:		ʶ������ʽ
	* result_encoding:	��������ʽ
	*
	*/
	const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";

	/* �û���¼ */
	ret = MSPLogin(NULL, NULL, login_params); //��һ���������û������ڶ������������룬����NULL���ɣ������������ǵ�¼����	
	if (MSP_SUCCESS != ret) {
		printf("MSPLogin failed , Error code %d.\n", ret);
		return -1; //��¼ʧ�ܣ��˳���¼
	}


	int errcode;

	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	errcode = sr_init(&iat, session_begin_params, SR_MIC, DEFAULT_INPUT_DEVID, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed\n");
		return -1;
	}

	printf("InitVoice successed!\n");

	return 0;
}


/* demo recognize the audio from microphone */
void RecognizeVoice(char **pResult)
{
	int errcode;

	m_pResult = pResult;

	if (errcode = sr_start_listening(&iat)) {
		printf("start listen failed %d\n", errcode);
	}
	else {
		printf("start listen successed\n");
	}
}

/* demo recognize the audio from microphone */
void UninitVoice()
{
	sr_uninit(&iat);

	MSPLogout(); //�˳���¼

	printf("UninitVoice finished\n");
}
