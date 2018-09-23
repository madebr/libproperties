#ifndef __properties_H_
#define __properties_H_


#define SOURCE_FORMAT_PROPERTIES    "properties"
#define SOURCE_FORMAT_YAML          "yaml"
#define SOURCE_FORMAT_XML           "xml"
#define SOURCE_FORMAT_INI           "ini"


//  Fountion:   P_READ
//      �� source ��ȡ����
//  Parameters:
//      source  -   ����Դ
//      buf     -   ���ڴ�Ŷ�ȡ���ݵĻ���������ʼ��ַ
//      size    -   ���������ܴ�С
//  Returns:
//      -1  -   ��ʾ��ȡʧ��
//      0   -   ��ʾ�������������Ѿ�û�����ݿ��Զ�ȡ
//      >0  -   ��ʾ�ɹ���ȡ�������ֽڵ�����
typedef int     (*P_READ)(struct source_t* source, char* buf, int size);
typedef void    (*P_FREE)(struct source_t* source);
struct source_t
{
    void*   context;    //  ��������������
    P_READ  read;       //  ��������
    P_FREE  free;       //  �ͷ�����Դ
    char*   cache;      //  ������
    size_t  size;       //  ��������С
};


int     source_init_by_file     (struct source_t* source, char* filename);
int     source_init_by_string   (struct source_t* source, char* str, char* end);
int     source_init_by_envs     (struct source_t* source);
void    source_exit             (struct source_t* source);


struct properties_t;
struct properties_t*    properties_new          ();
void                    properties_del          (struct properties_t* p);
int                     properties_load         (struct properties_t* p, struct source_t* source, char* format);
char*                   properties_get          (struct properties_t* p, char* key, char* def);
void                    properties_set          (struct properties_t* p, char* key, char* val);
void                    properties_clear        (struct properties_t* p, char* key);
char*                   properties_find         (struct properties_t* p, char* key);
void                    properties_foreach      (struct properties_t* p, int (*callback)(void* context, char* key, char* val), void* context);

#endif//__properties_H_
