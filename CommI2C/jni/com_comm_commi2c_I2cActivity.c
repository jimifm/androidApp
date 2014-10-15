#include <jni.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <android/log.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "comm_comm_commi2c_I2cActivity.h"

#define CHIP_ADDR  0xa0   //mcu i2c addr
#define I2C_DEV   "/dev/i2c-1"	// register i2c B bus

#define LOG_TAG "i2c"		//android logcat
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static int read_eeprom(int fd, char buff[], int addr, int count)
{
			int res;
			int i;

			for(i=0; i<PAGE_SIZE; i++)
      	{
           buff[i]=0;
			}


			if(write(fd, &addr, 1) != 1)
				return -1;
			usleep(10000);
			res=read(fd, buff, count);
			LOGI("read %d byte at 0x%.2x\n", res, addr);
			for(i=0; i<PAGE_SIZE; i++)
			{
				LOGI("0x%.2x, ", buff[i]);
			}

			return res;
}

static int write_eeprom(int fd, char  buff[], int addr, int count)
{
			int res;
			int i;
			char sendbuffer[PAGE_SIZE+1];

			memcpy(sendbuffer+1, buff, count);
			sendbuffer[0]=addr;

			res= write(fd, sendbuffer, count+1);
				LOGI("write %d byte at 0x%.2x\n", res ,addr);
			usleep(10000);

			for(i=0; i<PAGE_SIZE; i++)
			{
				LOGI("0x%.2x, ", buff[i]);
			}
}


JNIEXPORT jint JNICALL Java_com_comm_commi2c_I2cActivity_open(JNIEnv *env, jobject obj, jstring file)
{
      char fileName[64];
      const jbyte *str;

      str = (*env)->GetStringUTFChars(env, file, NULL);
      if (str == NULL) {
          LOGI("Can't get file name!");
          return -1;
      }
      sprintf(fileName, "%s", str);
    	  LOGI("will open i2c device node %s", fileName);
      (*env)->ReleaseStringUTFChars(env, file, str);
      return open(fileName, O_RDWR);
}

JNIEXPORT jint JNICALL Java_com_comm_commi2c_I2cActivity_read(JNIEnv * env, jobject obj, jint fileHander, jint slaveAddr, jintArray bufArr, jint len)
{
      jint *bufInt;
      char *bufByte;
      int res = 0, i = 0, j = 0;

      if (len <= 0) {
          LOGE("I2C: buf len <=0");
          goto err0;
      }

      bufInt = (jint *) malloc(len * sizeof(int));
      if (bufInt == 0) {
          LOGE("I2C: nomem");
          goto err0;
      }
      bufByte = (char*) malloc(len);
      if (bufByte == 0) {
          LOGE("I2C: nomem");
          goto err1;
      }

      (*env)->GetIntArrayRegion(env, bufArr, 0, len, bufInt);

      memset(bufByte, '\0', len);
      if ((j = read(fileHander, bufByte, len)) != len) {
        LOGE("read fail in i2c read jni i = %d buf 4", i);
        goto err2;
    } else

    {
        for (i = 0; i < j ; i++)
            bufInt[i] = bufByte[i];
        LOGI("return %d %d %d %d in i2c read jni", bufByte[0], bufByte[1], bufByte[2], bufByte[3]);
        (*env)->SetIntArrayRegion(env, bufArr, 0, len, bufInt);
    }

    free(bufByte);
    free(bufInt);

    return j;

err2:
    free(bufByte);
err1:
    free(bufInt);
err0:
    return -1;
}

JNIEXPORT jint JNICALL Java_com_comm_commi2c_I2cActivity_write(JNIEnv *env, jobject obj, jint fileHander, jint slaveAddr, jint mode, jintArray bufArr, jint len)
{
#if 1
      jint *bufInt;
      char *bufByte;
      int res = 0, i = 0, j = 0;

      if (len <= 0) {
          LOGE("I2C: buf len <=0");
          goto err0;
      }

      bufInt = (jint *) malloc(len * sizeof(int));
      if (bufInt == 0) {
          LOGE("I2C: nomem");
          goto err0;
      }
      bufByte = (char*) malloc(len + 1);
      if (bufByte == 0) {
          LOGE("I2C: nomem");
          goto err1;
      }

      (*env)->GetIntArrayRegion(env, bufArr, 0, len, bufInt);
      bufByte[0] = mode;
      for (i = 0; i < len; i++)
          bufByte[i + 1] = bufInt[i];

      if ((j = write(fileHander, bufByte, len + 1)) != len + 1) {
        LOGE("write fail in i2c");
        goto err2;
    }

    LOGI("I2C: write %d byte", j);
    free(bufByte);
    free(bufInt);

    return j - 1;

err2:
    free(bufByte);
err1:
    free(bufInt);
err0:
    return -1;

#endif
}

JNIEXPORT void JNICALL Java_com_comm_commi2c_I2cActivity_close(JNIEnv *env, jobject obj, jint fileHander)
{
      close(fileHander);
}
