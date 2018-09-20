

struct properties_t
{

};


struct p_cache_t
{
    int     len;
    char    cache[1024];
};

#define P_NEWLINE   (0x01)  //  \n  \0
#define P_VISIBLE   (0x02)  //  
#define P_SPLITER   (0x04)  //  : = \b
#define P_ESCAPE    (0x08)  //  \   
#define P_SPACE     (0x10)  //  \r  \t \b   ע�ⲻ�������У���Ϊ������Ҫ���⴦���к�����
#define P_COMMENT   (0x20)  //  #   !
unsigned char cmap[256] = 
{

};


struct p_cache_t
{
    int     lino;
    char*   line;   //  �е���ʼλ��
    char*   pos;    //  ��ǰ�Ѿ�ʶ��ʲôλ��
    char*   cache;  //  ��������С
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

    //  ������ǻ��б�ǣ���ô������ǰ�����ˣ������������������
    if (!(P_NEWLINE&cmap[*pos]))
    {
        return;
    }

    //  ������������滻�У���ô��Ҫ�����е���ʼλ�ú��кţ�Ȼ������ɨ��������հ�
    while ('\n' == *pos)
    {
        pos++;
        cache->pos = pos;
        cache->lino++;
        cache->line = pos;
        goto    retry;
    }

    //  ����������� \0 ���ţ���ô���ǿ�����Ϊ���������˻���������������������ֹ������
    if ('\0' == *pos)
    {
        //  ����ǻ���������
        if (pos == cache->end)
        {
            cache->pos = pos
            p_cache_read_more();    //  �����������
            pos = cache->pos;
            if (pos == cache->end)  //  �����������֮����Ȼ����
            {
                return;
            }

            //  ����������Ѿ�������������ˣ���ô����ǰ��Ĵ���
            goto    retry;
        }

        //  ��������ַ������� \0����ô�͵����������Ŵ����÷���ͳһ��key��ʶ���лᵱ��������
        return;
    }

    //  ����Ӧ���ߵ�������
    return;
}


inline static void p_accept_comment(struct p_cache_t* cache)
{
    register char* pos = cache->pos;

    //  ֻҪ���ǻ��л���cache��β����
    while (!(P_NEWLINE&cmap[*pos]))
    {
        pos++;
    }

    //  ��������滻��,���ﲻ�������ǽ������д�����ƴ���
    if ('\n' == *pos)
    {
        cache->pos = pos;
    }

    return;
}


char* p_accept_key(char* s)
{
    char* pos = s;

retry:
    //  �������еĿհ�
    while (P_SPACE&cmap[*pos])
    {
        pos++;
        continue;
    }

    //  �����һ���ǿհ��ַ���ע�ͣ����������ۺ���һ����㣬������ܣ�
    if ((P_COMMENT|P_EOS)&cmap[*pos])
    {
        if (P_COMMENT&cmap[*pos])
        {
            //  �����հ�����ֱ��\n����
            p_accept_comment()
            goto    retry;
        }

        
        //  ���һ���Ƿ���ĵ�\0�ˣ�����˵ֻ�ǻ���������
        goto    retry;
    }

    register char c = pos[0];
    
    read_key();

    
}

struct properties_t*    properties_new          ()
{

}


void                    properties_del          (struct properties_t* p)
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


