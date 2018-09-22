#include "properties.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct properties_t
{
    int a;
};


enum
{
    P_NEWLINE   =   (0x01), //  \n  \0
    P_SPLITER   =   (0x04), //  : = \b
    P_ESCAPE    =   (0x08), //  \   
    P_SPACE     =   (0x10), //  \r  \t \b   ע�ⲻ�������У���Ϊ������Ҫ���⴦���к�����
    P_COMMENT   =   (0x20), //  #   !
    P_EOS       =   (0x40), //  \0 
};


unsigned char cmap[256] = 
{
    0,
};

typedef int (*P_READ)(void* context, char* buf, int size);

struct p_cache_t
{
    int     lino;
    char*   line;       //  �е���ʼλ��
    char*   pos;        //  ��ǰ�Ѿ�ʶ�𵽵�λ��
    char*   end;        //  cache �Ľ�β


    char*   cache;      //  ����������ʼλ��
    char*   tail;       //  �������Ľ���λ��
    
    void*   context;
    P_READ  read;
};


//  �ļ��Ѿ����������ļ���ȡ�Ѿ�������
int p_read_from_file(void* context, char* buf, int size)
{
    FILE* file = (FILE*)context;
    if (feof(file))
    {
        return  0;
    }

    if (ferror(file))
    {
        return -1;
    }

    //  ��ʼ��ȡ
    size_t read_size   = fread(buf, 1, size, file);
    if (read_size > 0)
    {
        return read_size;
    }

    return read_size;
}

int p_read_from_string(void* context, char* buf, int size)
{
    return  0;
}


inline static char* p_cache_read_more(struct p_cache_t* cache, char* pos)
{
    //  ������ֹλ��
    cache->pos = pos;

    //  ������������滹�����ݴ��ڣ���ô��Ҫ�������ݣ��Ա��ڳ�����Ŀռ�
    int offset    = cache->pos - cache->cache;  //  ��Ҫ�ƶ���Զ
    if (offset > 0)
    {
        int move_size = cache->end - cache->pos;    //  ��Ҫ�ƶ���������
        if (move_size > 0)
        {
            memmove(cache->cache, pos, move_size);
        }
        cache->line -= offset;
        cache->pos  -= offset;
        cache->end  -= offset;
    }
    
    //  ��ȡ��������仺����
    int remain_size = cache->tail - cache->end;
    int ret = cache->read(cache->context, cache->end, remain_size);

    //  �ļ��Ѿ����������ļ���ȡ�Ѿ�������
    if (ret <= 0)
    {
        return cache->pos;
    }

    cache->end += ret;
    return cache->pos;
}




inline static void p_skip_space(struct p_cache_t* cache)
{
    register char* pos = cache->pos;

retry:
    //  �������еĿհ�
    while (P_SPACE&cmap[*pos])
    {
        pos++;
    }

    //  ����������� \0 ���ţ���ô���ǿ�����Ϊ���������˻���������������������ֹ������
    if ('\0' == *pos)
    {
        //  ����ǻ�������β���
        if (pos == cache->end)
        {
            //  �����������
            pos = p_cache_read_more(cache, pos);
            
            //  �����������֮����Ȼ�ǻ�������β��˵���Ѿ�����ȫ�����������
            if (pos == cache->end)
            {
                return;
            }

            //  ����������Ѿ�������������ˣ���ô����ǰ��Ĵ���
            goto    retry;
        }
    }

    //  ���������ȷ���Ѿ� cache->pos һ�����ǿհ���  
    return;
}


inline static void p_accept_comment(struct p_cache_t* cache)
{
    register char* pos = cache->pos;

retry:

    //  ע�Ϳ��ܻ���Ϊ�������������߶����ļ�β������
    while (!((P_NEWLINE|P_EOS)&cmap[*(++pos)]))
    {
    }

    //  �����������,��ôע�;ͽ�����
    if (P_NEWLINE&cm[*pos])
    {
        cache->pos = pos;
        return;
    }

    if (P_EOS&cm[*pos])
    {
        //  �����������
        pos = p_cache_read_more(cache, pos);

        //  �����������֮����Ȼ�ǻ�������β��˵���Ѿ�����ȫ�����������
        if (pos == cache->end)
        {
            return;
        }

        //  ����������Ѿ�������������ˣ���ô����ǰ��Ĵ���
        goto    retry;
    }

    return;
}


void p_accept_key(struct p_cache_t* cache)
{
    char* pos = s;

retry:
    
    while (!(P_SPACE|P_NEWLINE|P_ESCAPE|P_EOS|P_SPLITER)&cmap[*pos])
    {
        pos++;
        continue;
    }

    switch (cm[*pos])
    {
    case P_SPACE:
    case P_SPLITER:
        //  ����key
        cache->pos = pos;
        return;
    case P_NEWLINE:
        //  ����key������props���key
        cache->pos = pos;
        return;
    case P_ESCAPE:
        p_accept_escape(cache);
        goto    retry;
    case P_EOS:
        //  ���ʵ���ǻ�����������־
        if (pos == cache->end)
        {
            //  TODO ������Ҫ���ַ����浽cache����ȥ
            p_cache_read_more(cache);
            if (pos == cache->end)
            {
                //  ����key������props���key
                cache->pos = pos;
                return;
            }

            goto    retry;
        }

        //  ������ \0 ʵ���Ѿ��޷�������ȥ��
        goto    retry;
    default:
        //  ��Ӧ���ߵ�������
        ASSERT(0)
    }

    //  ��Ӧ���ߵ�������
    ASSERT(0)
}

inline static void p_accept_unicode_escape(struct p_cache_t* cache)
{
    //  \uxxxx
    cache->pos += 2;

    //  �����ǰ����
    if ((cache->pos + 4) > cache->end)
    {
        p_cache_read_more(cache, cache->pos);    //  ȷ��������4���ַ�
        if ((cache->pos + 4) > cache->end)
        {
            //  Ҫ�����𣿣���
            return;
        }
    }

    //  �����ݽ��и�ʽת��
}

inline static void p_accept_escape(struct p_cache_t* cache)
{
    cache->pos++;

retry:

    switch (*pos)
    {
    case '\0':
        //  ���ʵ���ǻ�����������־
        if (pos == cache->end)
        {
            //  TODO    ��ʱҪ���ַ���������
            pos = p_cache_read_more(cache, pos);
            if (pos == cache->end)
            {
                return;
            }

            goto    retry;
        }

        //  ������ \0 ʵ���Ѿ��޷�������ȥ��
        goto    retry;
    case 'n':
        //  TODO \n
        return;
    case 't':
        //  TODO \t
        return;
    case 'r':
        //  TODO \t
        return;
    case 'u':
        register char* pos = s;
        //unicodeת��
        cache->pos = pos;
        p_accept_unicode_escape(cache);
        return;
    default:
        // *pos
        return;
    }
}




inline static void p_accept_value(struct p_cache_t* cache)
{
    register char* pos = s;

retry:

    while (!(P_NEWLINE|P_ESCAPE|P_EOS)&cmap[*pos])
    {
        pos++;
        continue;
    }

    switch (cm[*pos])
    {
    case P_NEWLINE:
        //  ����key������props���key
        cache->pos = pos;
        return;
    case P_ESCAPE:
        p_accept_escape(cache);
        goto    retry;
    case P_EOS:
        //  ���ʵ���ǻ�����������־
        if (pos == cache->end)
        {
            //  TODO    ��ʱҪ���ַ���������
            pos = p_cache_read_more(cache, pos);
            if (pos == cache->end)
            {
                //  ����key������props���key
                cache->pos = pos;
                return;
            }

            goto    retry;
        }

        //  ������ \0 ʵ���Ѿ��޷�������ȥ��
        goto    retry;
    default:
        //  ��Ӧ���ߵ�������
        ASSERT(0)
    }

    //  ��Ӧ���ߵ�������
    ASSERT(0)
}

void p_accept_spliter(struct p_cache_t* cache)
{
    register char* pos = cache->pos;

retry:

    while (!((P_SPACE|P_SPLITER|P_EOS|P_NEWLINE)&cmp[*pos]))
    {
        pos++;
    }
    cache->pos = pos;

    if (P_EOS&cmp[*pos])
    {
        //  ���ʵ���ǻ�����������־
        if (pos == cache->end)
        {
            p_cache_read_more(cache);
            if (pos == cache->end)
            {
                return;
            }

            goto    retry;
        }

        //  ������ \0 ʵ���Ѿ��޷�������ȥ��
        return;
    }

    if ((P_SPACE|P_SPLITER)&cmp[pos])
    {
        p_skip_space(cache);
        return;
    }

    if (P_NEWLINE&cmp[pos])
    {
        return;
    }
}

int properties_parse(struct p_cache_t* cache, void* context)
{
    register char* pos = cache->pos;

retry:

    p_skip_space(cache);
    pos = cache->pos;

    //  �����ע���У�����
    if (P_COMMENT&cmp[*pos])
    {
        p_accept_comment(cache);
        goto    retry;
    }

    //  ����ǿ��У��޸��кź�����ʼ
    if (P_NEWLINE&cmp[*pos])
    {
        cache->pos++;
        cache->line = cache->pos;
        cache->lino++;
        goto    retry;
    }

    //  �������\0���ţ��������������β����ôʶ�����
    if (P_EOS&cmp[*pos])
    {
        //  �������β��˵���Ѿ��޷�������������ݣ�ʶ�����
        if (pos == cache->end)
        {
            return 0;
        }
    }

    p_accept_key(cache);
}






struct properties_t*    properties_new         void                    properties_del          (struct properties_t* p)
{

}


int                     properties_load_file    (struct properties_t* p, char* file)
{

}


int                     properties_load_string  (struct properties_t* p, char* s)
{
    if (NULL == s)
    {
        return -1;
    }

    struct p_cache_t cache;
    cache
    p_accept_key()
    
}


int                     properties_load_envs    (struct properties_t* p, char* file)
{

}


char*                   properties_get          (struct properties_t* p, char* key, char* def)
{

}


void                    properties_set          (struct properties_t* p, char* key, char* val)
{

}


void                    properties_clear        (struct properties_t* p, char* key)
{

}


char*                   properties_find         (struct properties_t* p, char* key)
{

}


void                    properties_foreach      (struct properties_t* p, int (*callback)(void* context, char* key, char* val), void* context)
{

}


