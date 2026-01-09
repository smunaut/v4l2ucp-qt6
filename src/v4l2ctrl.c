/*  v4l2ctrl - A program for saving and loading settings for V4L2 devices
    Copyright (C) 2008-2009 Scott J. Bertin (scottbertin@yahoo.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */
#include <errno.h>
#include <fcntl.h>
#include <libv4l2.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FORMATW "%u:%31s:%d\n"
#define FORMATR "%u:%31c:%d\n"

void usage(const char *argv0) {
    printf("Usage: %s [-d device] -s filename\n", argv0);
    printf("       %s [-d device] -l filename\n", argv0);
    printf("       %s -h\n", argv0);
    printf("-s to save settings to filename\n");
    printf("-l to load settings from filename\n");
    printf("-d to specify the device name to use. Defaults to /dev/video0.\n");
    printf("-h to print this message.\n");
}

int do_save(int fd, FILE *file) {
    int i;
    struct v4l2_queryctrl ctrl;
    struct v4l2_control c;

#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
    /* Try the extended control API first */
    ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    if (0 == v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
        do {
            c.id = ctrl.id;
            ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
            if (ctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                continue;
            }
            if (ctrl.type != V4L2_CTRL_TYPE_INTEGER && ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
                ctrl.type != V4L2_CTRL_TYPE_MENU) {
                continue;
            }
            if (v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == 0) {
                fprintf(file, FORMATW, c.id, ctrl.name, c.value);
            }
        } while (0 == v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl));
    } else
#endif
    {
        /* Check all the standard controls */
        for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
            ctrl.id = i;
            if (v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                if (ctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                    continue;
                }
                if (ctrl.type != V4L2_CTRL_TYPE_INTEGER && ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
                    ctrl.type != V4L2_CTRL_TYPE_MENU) {
                    continue;
                }
                c.id = i;
                if (v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == 0) {
                    fprintf(file, FORMATW, i, ctrl.name, c.value);
                }
            }
        }

        /* Check any custom controls */
        for (i = V4L2_CID_PRIVATE_BASE;; i++) {
            ctrl.id = i;
            if (v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                if (ctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                    continue;
                }
                if (ctrl.type != V4L2_CTRL_TYPE_INTEGER && ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
                    ctrl.type != V4L2_CTRL_TYPE_MENU) {
                    continue;
                }
                c.id = i;
                if (v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == 0) {
                    fprintf(file, FORMATW, i, ctrl.name, c.value);
                }
            } else {
                break;
            }
        }
    }

    return 0;
}

int do_load(int fd, FILE *file) {
    __u32 id;
    __s32 value;
    struct v4l2_queryctrl ctrl;
    struct v4l2_control c;
    char name[sizeof(ctrl.name)], *n;

    name[sizeof(ctrl.name) - 1] = 0;
    while (fscanf(file, FORMATR, &id, name, &value) == 3) {
        n = name;
        while (*n == ' ') {
            n++;
        }
        ctrl.id = id;
        if (v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
            if (strcmp((char *)ctrl.name, n)) {
                fprintf(stderr, "Control name mismatch\n");
                return EXIT_FAILURE;
            }

            if (ctrl.flags & (V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_GRABBED)) {
                continue;
            }
            if (ctrl.type != V4L2_CTRL_TYPE_INTEGER && ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
                ctrl.type != V4L2_CTRL_TYPE_MENU) {
                continue;
            }

            c.id = id;
            c.value = value;
            if (v4l2_ioctl(fd, VIDIOC_S_CTRL, &c) != 0) {
                fprintf(stderr, "Failed to set control \"%s\": %s\n", ctrl.name, strerror(errno));
                continue;
            }
        } else {
            fprintf(stderr, "Error querying control %s: %s\n", n, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    if (!feof(file)) {
        fprintf(stderr, "Error reading from file\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int i, fd, ret;
    int load = -1;
    const char *device = "/dev/video0";
    const char *filename, *mode;
    FILE *file;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d") && i < argc - 1) {
            device = argv[++i];
        } else if (!strcmp(argv[i], "-s") && i < argc - 1) {
            filename = argv[++i];
            load = 0;
        } else if (!strcmp(argv[i], "-l") && i < argc - 1) {
            filename = argv[++i];
            load = 1;
        } else if (!strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (load < 0) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    fd = v4l2_open(device, O_RDWR, 0);
    if (fd < 0) {
        fprintf(stderr, "Unable to open %s: %s\n", device, strerror(errno));
        return EXIT_FAILURE;
    }

    if (load) {
        mode = "r";
    } else {
        mode = "w";
    }

    file = fopen(filename, mode);
    if (!file) {
        fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
        v4l2_close(fd);
        return EXIT_FAILURE;
    }

    if (load) {
        ret = do_load(fd, file);
    } else {
        ret = do_save(fd, file);
    }

    fclose(file);
    v4l2_close(fd);

    return ret;
}
