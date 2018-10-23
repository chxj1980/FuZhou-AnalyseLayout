// test1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <zmq.hpp>
#include <zhelpers.hpp>
#include "czmq.h"

//#define  USE_PROXY 0

#define LRU_READY	"\001"	//worker�Ѿ���
#pragma comment( lib, "libzmq-v100-mt-gd-4_0_4.lib" )
int main(int argc, char* argv[])
{

    zctx_t *ctx = zctx_new();
    void *frontend = zsocket_new(ctx, ZMQ_ROUTER);
    void *backend = zsocket_new(ctx, ZMQ_ROUTER);
    zsocket_bind(frontend, "tcp://*:5555");
    zsocket_bind(backend, "tcp://*:5556");

    zlist_t *workers = zlist_new();

    while (1)
    {
        zmq_pollitem_t items[] =
        {
            { backend, 0, ZMQ_POLLIN, 0 },
            { frontend, 0, ZMQ_POLLIN, 0 }
    };
        //	�����õ�workerʱ����ѯǰ�˶˵�
        int rc = zmq_poll(items, zlist_size(workers) ? 2 : 1, -1);
        if (rc == -1)
        {
            break;
        }


        //	�����˶˵��worker��Ϣ
        if (items[0].revents & ZMQ_POLLIN)
        {
            //	ʹ��worker�ĵ�ַ����LRU�Ŷ�
            zmsg_t *msg = zmsg_recv(backend);
            if (!msg)
            {
                break;
            }
            zframe_t *address = zmsg_unwrap(msg);
            zlist_append(workers, address);
            printf("worker size = %d\n", zlist_size(workers));

            //	�����Ϣ����READY����ת����Client
            zframe_t *frame = zmsg_first(msg);
            if (memcmp(zframe_data(frame), LRU_READY, 1) == 0)
            {
                zmsg_destroy(&msg);
            }
            else
            {
                zmsg_send(&msg, frontend);
            }
        }

        if (items[1].revents & ZMQ_POLLIN)
        {
            //	��ȡclient����ת������һ�����õ�worker
            zmsg_t *msg = zmsg_recv(frontend);
            if (msg)
            {
                zmsg_wrap(msg, (zframe_t*)zlist_pop(workers));
                zmsg_send(&msg, backend);
                printf("worker size = %d\n", zlist_size(workers));
            }

        }
}

    while (zlist_size(workers))
    {
        zframe_t *frame = (zframe_t *)zlist_pop(workers);
        zframe_destroy(&frame);
    }

    zlist_destroy(&workers);
    zctx_destroy(&ctx);
    return 0;
}


