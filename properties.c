

struct properties_t
{

};


struct p_cache_t
{
    int     len;
    char    cache[1024];
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

};


struct p_cache_t
{
    int     lino;
    char*   line;   //  �е���ʼλ��
    char*   end;    //  cache �Ľ�β
};

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
    if (P_NEWLINE&cmp[*pos])
    {
        cache->pos = pos;
        return;
    }

    if (P_EOS&cmp[*pos])
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

    switch (cmp[*pos])
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
        p_cache_read_more(cache, cache->pos, 4);    //  ȷ��������4���ַ�
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

    switch (cmp[*pos])
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

int properties_parse(struct p_cache_t* cache)
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


