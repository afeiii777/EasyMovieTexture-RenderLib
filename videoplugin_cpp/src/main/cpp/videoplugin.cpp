#include <jni.h>
#include <string>

#include <android/log.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include "logutils.h"

unsigned int FBO, VAO, VBO;

unsigned int framTexture, videoTexture;

unsigned int shaderProgame;

unsigned int vertextShder, fragmentShder;

int framWidth, framHeight;

char* vertexShaderSource =
"#version 300 es                                 \n"
"layout (location = 0) in vec2 a_Position;       \n"
"layout (location = 1) in vec2 a_TexCoord;       \n"
"out vec2 fragmentTexCoord;                      \n"
"void main() {                                   \n"
"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"   fragmentTexCoord = a_TexCoord;               \n"
"}                                               \n";

char* fragmentShaderSource =
"#version 300 es                                         \n"
"#extension GL_OES_EGL_image_external_essl3 : require    \n"
"precision mediump float;                                \n"
"in vec2 fragmentTexCoord;                               \n"
"out vec4 fragColor;                                     \n"
"uniform samplerExternalOES videoTexture;                \n"
"void main() {                                           \n"
"   fragColor = texture(videoTexture, fragmentTexCoord); \n"
"}                                                       \n";

float vertices[] = {

        -1, 1,0, 0,
        1, 1,1, 0,
        -1, -1,0, 1,
        1, -1,1, 1,
};


void setUniform(const char* name,int i){
    int temp = glGetUniformLocation(shaderProgame, name);
    glUniform1i(temp,i);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_pvr_videoplugin_VideoPlugin_createOESTextureIDD(JNIEnv *env, jobject thiz) {
    LOGI("%s", __func__);
    glGenTextures(1,&videoTexture);
    LOGI("????%d",videoTexture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,videoTexture);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_EXTERNAL_OES);
    return (jint)videoTexture;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_pvr_videoplugin_VideoPlugin_renderInit(JNIEnv *env, jobject thiz, jint width, jint height,
                                                jint texture_id) {
    LOGI("%s", __func__);

    framWidth = width;
    framHeight = height;


    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4,(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4,(void*)(2 * sizeof(float)));

    glGenFramebuffers(1,&FBO);
    glBindFramebuffer(GL_FRAMEBUFFER,FBO);

    framTexture = texture_id;
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,framTexture,0);

    shaderProgame = glCreateProgram();

    vertextShder = glCreateShader(GL_VERTEX_SHADER);
    fragmentShder = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertextShder,1,&vertexShaderSource,NULL);
    glShaderSource(fragmentShder,1,&fragmentShaderSource,NULL);
    glCompileShader(vertextShder);
    glCompileShader(fragmentShder);
    glAttachShader(shaderProgame,vertextShder);
    glAttachShader(shaderProgame,fragmentShder);
    glLinkProgram(shaderProgame);
    glDeleteShader(fragmentShder);
    glDeleteShader(vertextShder);

    glViewport(0,0,framWidth,framHeight);
    setUniform("videoTexture",0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_pvr_videoplugin_VideoPlugin_renderDraw(JNIEnv *env, jobject thiz) {
    //LOGI("%s", __func__);

    glUseProgram(shaderProgame);


    glClearColor(1.0,0,0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER,FBO);

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,videoTexture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_pvr_videoplugin_VideoPlugin_renderRelease(JNIEnv *env, jobject thiz) {

    glBindTexture(GL_TEXTURE_2D,0);
    glDeleteTextures(1,&framTexture);
    glDeleteTextures(1,&videoTexture);
    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteFramebuffers(1,&FBO);
    glDeleteProgram(shaderProgame);
}