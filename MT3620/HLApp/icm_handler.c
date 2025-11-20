// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#include "icm_handler.h"
#include <applibs/log.h>
#include <applibs/application.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static int listen_fd = -1;
static int conn_fd = -1;

bool ICM_Initialize(void) {
    Log_Debug("ICM: Initializing inter-core messaging\n");
    
    // Create listening socket for ICM communication with RTApp
    listen_fd = Application_Socket(0); // Listen for connections
    
    if (listen_fd < 0) {
        Log_Debug("ERROR: Failed to create ICM listening socket (errno=%d)\n", errno);
        return false;
    }
    
    Log_Debug("ICM: Waiting for RTApp connection...\n");
    
    // Accept connection from RTApp
    conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd < 0) {
        Log_Debug("ERROR: Failed to accept RTApp connection (errno=%d)\n", errno);
        close(listen_fd);
        listen_fd = -1;
        return false;
    }
    
    Log_Debug("ICM: Connected to RTApp\n");
    return true;
}

void ICM_Cleanup(void) {
    Log_Debug("ICM: Cleaning up\n");
    
    if (conn_fd >= 0) {
        close(conn_fd);
        conn_fd = -1;
    }
    
    if (listen_fd >= 0) {
        close(listen_fd);
        listen_fd = -1;
    }
}

bool ICM_ReceiveTelemetry(void *buffer, size_t buffer_size, size_t *received_size) {
    if (conn_fd < 0 || buffer == NULL || buffer_size == 0) {
        return false;
    }
    
    // Non-blocking receive
    ssize_t received = recv(conn_fd, buffer, buffer_size, MSG_DONTWAIT);
    
    if (received > 0) {
        if (received_size) {
            *received_size = (size_t)received;
        }
        
        Log_Debug("ICM: Received telemetry (%zd bytes)\n", received);
        return true;
    } else if (received == 0) {
        // Connection closed
        Log_Debug("WARNING: ICM connection closed by RTApp\n");
        return false;
    } else {
        // No data available (EAGAIN/EWOULDBLOCK) or error
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Log_Debug("ERROR: ICM receive error (errno=%d)\n", errno);
        }
        return false;
    }
}

bool ICM_SendConfiguration(const void *data, size_t size) {
    if (conn_fd < 0 || data == NULL || size == 0) {
        return false;
    }
    
    ssize_t sent = send(conn_fd, data, size, 0);
    
    if (sent == (ssize_t)size) {
        Log_Debug("ICM: Sent configuration command (%zu bytes)\n", size);
        return true;
    } else {
        Log_Debug("ERROR: Failed to send configuration (sent=%zd, expected=%zu)\n",
                  sent, size);
        return false;
    }
}
