#include "properties.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifndef ASSERT
#define ASSERT(e)   assert(e)
#endif//ASSERT


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


unsigned char cm[256] = 
{
    0,
};




struct cache_t
{
    struct source_t*    source;
    int                 outsize;    //  cache�Ƿ������ⲿ
    char*               cache;      //  ����������ʼλ��
    char*               tail;       //  �������Ľ���λ��

    int                 lino;       //  �к�
    char*               line;       //  �е���ʼλ��
    char*               pos;        //  ��ǰ�Ѿ�ʶ�𵽵�λ��
    char*               end;        //  cache �Ľ�β
};




static void p_source_file_free(struct source_t* source)
{
    if (NULL == source)
    {
        return;
    }

    if (NULL != source->context)
    {
        fclose((FILE*)(source->context));
    }
}

//  �ļ��Ѿ����������ļ���ȡ�Ѿ�������
static int p_source_file_read(struct source_t* source, char* buf, int size)
{
    FILE* file = (FILE*)(source->context);
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

int source_init_by_file     (struct source_t* source, char* filename)
{
    ASSERT(NULL != source);
    ASSERT(NULL != filename);

    FILE* file = fopen(filename, "r");
    if (NULL == file)
    {
        return  -1;
    }

    memset(source, 0, sizeof(struct source_t));
    source->read    =   p_source_file_read;   
    source->free    =   p_source_file_free;
    source->context =   file;
    source->cache   =   NULL;
    source->size    =   0;

    return  0;
}

static void p_source_string_free(struct source_t* source)
{
    return;
}

static int p_source_string_read(struct source_t* source, char* buf, int size)
{
    return 0;
}

int source_init_by_string   (struct source_t* source, char* str, char* end)
{
    ASSERT(NULL != source);
    ASSERT(NULL != str);
    if (NULL == end)
    {
        end = str + strlen(str);
    }

    memset(source, 0, sizeof(struct source_t));
    source->read    =   p_source_string_read;   
    source->free    =   p_source_string_free;
    source->context =   NULL;
    source->cache   =   str;
    source->size    =   end - str;

    return  0;
}

int source_init_by_envs     (struct source_t* source)
{

}

void                    source_exit             (struct source_t* source)
{
    if (NULL == source)
    {
        return;
    }

    source->free(source);
}


inline static char* p_cache_read_more(struct cache_t* cache, char* pos)
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
    int ret = cache->source->read(cache->source, cache->end, remain_size);

    //  �ļ��Ѿ����������ļ���ȡ�Ѿ�������
    if (ret <= 0)
    {
        return cache->pos;
    }

    cache->end += ret;
    return cache->pos;
}




inline static void p_skip_space(struct cache_t* cache)
{
    register char* pos = cache->pos;

retry:
    //  �������еĿհ�
    while (P_SPACE&cm[*pos])
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


inline static void p_accept_comment(struct cache_t* cache)
{
    register char* pos = cache->pos;

retry:

    //  ע�Ϳ��ܻ���Ϊ�������������߶����ļ�β������
    while (!((P_NEWLINE|P_EOS)&cm[*(++pos)]))
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

inline static void p_accept_unicode_escape(struct cache_t* cache)
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



inline static void p_accept_escape(struct cache_t* cache)
{
    cache->pos++;

    register char* pos = cache->pos;

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
        //unicodeת��
        cache->pos = pos;
        p_accept_unicode_escape(cache);
        return;
    default:
        // *pos
        return;
    }
}


void p_accept_key(struct cache_t* cache)
{
    char* pos = cache->pos;

retry:
    
    while (!(P_SPACE|P_NEWLINE|P_ESCAPE|P_EOS|P_SPLITER)&cm[*pos])
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
        ASSERT(0);
    }

    //  ��Ӧ���ߵ�������
    ASSERT(0);
}


inline static void p_accept_value(struct cache_t* cache)
{
    register char* pos = cache->pos;

retry:

    while (!(P_NEWLINE|P_ESCAPE|P_EOS)&cm[*pos])
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
        ASSERT(0);
    }

    //  ��Ӧ���ߵ�������
    ASSERT(0);
}

void p_accept_spliter(struct cache_t* cache)
{
    register char* pos = cache->pos;

retry:

    while (!((P_SPACE|P_SPLITER|P_EOS|P_NEWLINE)&cm[*pos]))
    {
        pos++;
    }
    cache->pos = pos;

    if (P_EOS&cm[*pos])
    {
        //  ���ʵ���ǻ�����������־
        if (pos == cache->end)
        {
            pos = p_cache_read_more(cache, pos);
            if (pos == cache->end)
            {
                return;
            }

            goto    retry;
        }

        //  ������ \0 ʵ���Ѿ��޷�������ȥ��
        return;
    }

    if ((P_SPACE|P_SPLITER)&cm[*pos])
    {
        p_skip_space(cache);
        return;
    }

    if (P_NEWLINE&cm[*pos])
    {
        return;
    }
}

int properties_load         (struct properties_t* p, struct source_t* source, char* format)
{
    ASSERT(NULL != p);
    ASSERT(NULL != source);
    ASSERT(NULL != format);

    struct cache_t cache;
    cache.source    =   source;
    if (NULL != source->cache)
    {
        cache.cache     =   source->cache;
        cache.tail      =   source->cache + source->size;
        cache.outsize   =   1;
    }
    else
    {
        cache.cache     =   (char*)malloc(CACHE_SIZE_DEF);
        cache.outsize   =   0;
    }
    cache.lino      =   0;
    cache.line      =   cache.cache;
    cache.pos       =   cache.cache;
    cache.end       =   cache.cache;

    switch (format[0])
    {
    case 'p':
        if (0 == strcmp(format, SOURCE_FORMAT_PROPERTIES))
        {
            return p_parse_format_properties(p, &cache);
        }
        return -1;
    case 'y':
        if (0 == strcmp(format, SOURCE_FORMAT_YAML))
        {
            return p_parse_format_yaml(p, &cache);
        }
        return -1;
    case 'x':
        if (0 == strcmp(format, SOURCE_FORMAT_XML))
        {
            return p_parse_format_xml(p, &cache);
        }
        return -1;
    case 'i':
        if (0 == strcmp(format, SOURCE_FORMAT_INI))
        {
            return p_parse_format_ini(p, &cache);
        }
        return -1;
    default:
        return -1;
    }
}
int p_parse_format_yaml(struct properties_t* p, struct cache_t* cache)
{
    return  -1;
}

int p_parse_format_xml(struct properties_t* p, struct cache_t* cache)
{
    return  -1;
}

int p_parse_format_ini(struct properties_t* p, struct cache_t* cache)
{
    return  -1;
}


int p_parse_format_properties(struct properties_t* p, struct cache_t* cache)
{
    register char* pos = cache->pos;

    while (1)
    {
        p_skip_space(cache);
        pos = cache->pos;

        //  �����ע���У�����
        if (P_COMMENT&cm[*pos])
        {
            p_accept_comment(cache);
            continue;
        }

        //  ����ǿ��У��޸��кź�����ʼ
        if (P_NEWLINE&cm[*pos])
        {
            cache->pos++;
            cache->line = cache->pos;
            cache->lino++;
            continue;
        }

        //  �������\0���ţ��������������β����ôʶ�����
        if (P_EOS&cm[*pos])
        {
            //  �������β��˵���Ѿ��޷�������������ݣ�ʶ�����
            if (pos == cache->end)
            {
                return 0;
            }
        }

        p_accept_key(cache);
    }
}
