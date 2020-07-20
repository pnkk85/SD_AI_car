#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h> 
#include <netinet/in.h> 

#include <linux/can.h>
#include <linux/can/raw.h>

#define IFNAME "slcan0"
#define PORT 8080

int main(void)
{
    int s_can;
    int nbytes_can;
    struct sockaddr_can addr_can;
    struct can_frame frame;
    struct ifreq ifr;
    int can_msg[2];

    /*
    *
    *	cocketCAN initialization
    *
    */
    if( (s_can = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1 )
    {
        perror("socketCAN");
        exit(1);
    }
    
    strcpy(ifr.ifr_name, IFNAME);
    ioctl(s_can, SIOCGIFINDEX, &ifr);

    addr_can.can_family = AF_CAN;
    addr_can.can_ifindex = ifr.ifr_ifindex;

    printf("%s at index %d\n", IFNAME, ifr.ifr_ifindex);

    if( bind(s_can, (struct sockaddr*)&addr_can, sizeof(addr_can))  == -1 )
    {
        perror("bind");
        exit(1);
    }

    frame.can_id = 0x123;
    frame.can_dlc = 4;
    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x11;
    frame.data[3] = 0x22;

    /*
    *
    * TCP server initialization
    *
    */
    int s_server, s_new, nbytes;
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char str[9];

    if ((s_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }

    if (setsockopt(s_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT );

    if (bind(s_server, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(s_server, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((s_new = accept(s_server, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    /* read can and send tcp message in endless loop */
    int i = 0;
    for(;;)
    {
        /* read can messages */
        nbytes_can = read(s_can, &frame, sizeof(struct can_frame));
        
        /* send over tcp every second message only */
        if(i == 1)
        {
            i = 0;
            can_msg[0] =  (frame.data[0] << 8 )|(frame.data[1]);
            can_msg[1] =  (frame.data[2] << 8 )|(frame.data[3]);
            snprintf(str, sizeof(str), "%d%d", can_msg[0], can_msg[1]);
            printf("\r%s", str);
            fflush(stdout);
            send(s_new, str, strlen(str), 0);
        }else{
            i = 1;
        }
        
    }

    return 0;
}
