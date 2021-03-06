/*
 * Copyright (c) 2014-2016, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "utils.h"

int count_char(char *src, char achar) {
    int num = 0;

    if(src == NULL)
        return -1;

    char *p = src;

    while(*p != '\0') {

        if(*p == achar)
            num++;

        p++;
    }

    return num;
}

/**
*  input: src string
*   line: start line num
*/
int get_pos(char *src, int line) {
    int num = 0, pos = 0;

    if(src == NULL)
        return -1;

    char *p = src;

    while(*p != '\0') {

        if(num == line)
            break;

        if(*p == '\n')
            num++;

        p++;
        pos++;
    }

    return pos;
}

int charArray_to_hexArray(char *input, int inputLen, int start, int len, char *output, int outputLen, bool revert) {
    if(inputLen < len || outputLen < len)
        return -1;

    for(int i = 0; i < len; i++) {
        char high = *(input + start + i) / 16;

        if(high >= 10)
            high = 'a' + high - 10;
        else
            high += '0';
        char low = *(input + start + i) % 16;

        if(low >= 10)
            low = 'a' + low - 10;
        else
            low += '0';
        if(revert) {
            *(output + i * 2) = low;
            *(output + i * 2 + 1) = high;
        } else {
            *(output + i * 2) = high;
            *(output + i * 2 + 1) = low;
        }
    }
    return 0;
}

int exe_cmd(callback cb, exec_cmd_t * exec_cmd) {
    int pipefd[2];
    int cpid, i = 0, j = 0, ret = FAILED;
    char line[SIZE_1K] = { 0 };

    if(exec_cmd == NULL) {
        MMI_ALOGE("input cmd is NULL");
        return ret;
    }

    if(pipe(pipefd) == -1) {
        MMI_ALOGE("create pipe file fail, error=%s\n", strerror(errno));
        return FAILED;
    }

    cpid = fork();
    if(cpid == -1) {
        MMI_ALOGE("fork fail, error=%s\n", strerror(errno));
        return FAILED;
    } else if(cpid == 0) {      /* Child stdout from pipe */
        close(pipefd[0]);

        if(dup2(pipefd[1], STDOUT_FILENO) == -1) {
            MMI_ALOGE("fail to redirect std output and exit, error=%s\n", strerror(errno));
            _exit(100);
        }

        if(dup2(pipefd[1], STDERR_FILENO) == -1) {
            MMI_ALOGE("fail to redirect std err and exit, error=%s\n", strerror(errno));
            _exit(100);
        }

        MMI_ALOGI("start exec command:'%s', pid=%d\n", exec_cmd->cmd, getpid());
        ret = execv(exec_cmd->cmd, exec_cmd->params);
        if(ret == FAILED) {
            MMI_ALOGE("execv command(%s) fail and exit, error=%s\n", exec_cmd->cmd, strerror(errno));
            _exit(100);
        }
    } else {                    /* Parent read */
        signal(SIGCHLD, SIG_IGN);
        *(exec_cmd->pid) = cpid;
        close(pipefd[1]);
        while((read(pipefd[0], &line[i], 1) > 0) && (j < exec_cmd->size)) {
            if(i < SIZE_1K - 1 && line[i++] == '\n') {
                /**ignore space line*/
                if(strlen(line) > 1)
                    strlcat(exec_cmd->result, line, exec_cmd->size);

                if(cb != NULL)
                    cb(exec_cmd->result, strlen(exec_cmd->result));

                /**Check if need to force exit*/
                if(exec_cmd->exit_str != NULL && strstr(line, exec_cmd->exit_str) != NULL) {
                    MMI_ALOGI("'%s' need to force exit\n", exec_cmd->cmd);
                    break;
                }

                memset(line, 0, sizeof(line));
                i = 0;
            }
            j++;
        }

        MMI_ALOGI("kill command '%s'(pid=%d)\n", exec_cmd->cmd, cpid);
        kill(cpid, SIGTERM);

        do {
            ret = waitpid(cpid, NULL, 0);
            if(cpid == ret) {
                MMI_ALOGI("'%s'(pid=%d) exit\n", exec_cmd->cmd, cpid);
            }
        } while(ret == FAILED && errno == EINTR);

        close(pipefd[0]);
    }

    return SUCCESS;
}

int get_device_index(char *device, const char *path, int *deviceIndex) {
    if(device == NULL || path == NULL) {
        MMI_ALOGE("Invalid parameter\n");
        return -1;
    }
    string input(device);
    string num;
    int index = input.find_first_of(path);

    if(index >= 0) {
        num = input.substr(strlen(path));
        *deviceIndex = (int) strtol(num.c_str(), NULL, 10);
        return 0;
    } else {
        MMI_ALOGE("Invalid device index, index=%d\n", index);
        return -1;
    }
}

void get_para_value(const unordered_map < string, string > paras, const char *key, char *value, int len,
                    const char *def) {
    unordered_map < string, string > parameters = paras;
    strlcpy(value, parameters[key].c_str(), len);
    if(strlen(value) == 0 && def != NULL) {
        strlcpy(value, def, len);
    }
}

void get_para_value(const unordered_map < string, string > paras, const char *key, string & value, const char *def) {
    unordered_map < string, string > parameters = paras;
    value = parameters[key];
    if(value.empty())
        value = (string) def;
}

void get_para_value(const unordered_map < string, string > paras, const string key, string & value, const char *def) {
    get_para_value(paras, key.c_str(), value, def);
}

char *trim(char *str) {
    char *p = str;

    if(*p == '\0') {
        return p;
    }

    while(*p == ' ' || *p == '\n' || *p == '\t') {
        p++;
    }
    if(*p == '\0') {
        return p;
    }

    char *end = p + strlen(p) - 1;

    while(*end == ' ' || *end == '\n' || *end == '\t') {
        *end = '\0';
        end--;
    }
    return p;
}

void trim(string & src) {
    const string delim = " \n\t";

    src.erase(src.find_last_not_of(delim) + 1);
    src.erase(0, src.find_first_not_of(delim));
}

void parse_parameter(const string src, unordered_map < string, string > &paras) {
    string key, value;

    paras.clear();
    if(src.length() < 3)
        return;

    int first = 0, next = 0;
    string sub;

    MMI_ALOGD("parse string: %s", src.c_str());
    while(1) {
        next = src.find_first_of(';', first);
        if(first > src.length() - 3)
            break;

        if(next < first)
            next = src.length();

        sub = src.substr(first, next - first);
        int index = sub.find(':');

        if(index < 0)
            break;
        if(index > 0 && index < (int) sub.length()) {
            key = sub.substr(0, index);
            trim(key);
            value = sub.substr(index + 1);
            trim(value);
            key.find_first_not_of(' ');
            paras[key] = value;

            MMI_ALOGD("parsed parameter: %s:%s", key.c_str(), value.c_str());
        }
        first = next + 1;
    }

}

void parse_parameter(const char *src, unordered_map < string, string > &paras) {
    if(src != NULL)
        parse_parameter(string(src), paras);
}

/*parser "name indicator value" format,eg:name=value */
int parse_value(const char *line, char indicator, char *name, int name_len, char *value, int value_len) {
    if(line == NULL || name == NULL || value == NULL)
        return -1;
    string input(line);
    int split_index = input.find_first_of(indicator);

    if(split_index > 0) {
        if(split_index < name_len && strlen(line) - split_index - 1 < value_len) {
            strlcpy(name, line, name_len);
            name[split_index] = '\0';
            strlcpy(value, line + split_index + 1, value_len);
            value[strlen(line) - split_index - 1] = '\0';
            return 0;
        } else
            return -1;
    } else
        return -1;
}

int parse_nv_by_indicator(const char *line, char indicator, char *name, int nameLen, char *value, int valueLen) {
    if(line == NULL || name == NULL || value == NULL)
        return -1;
    string input(line);
    int splitIndex = input.find_first_of(indicator);

    if(splitIndex > 0) {
        if(splitIndex < nameLen && (int) strlen(line) - splitIndex - 1 < valueLen) {
            strlcpy(name, line, nameLen);
            name[splitIndex] = '\0';
            strlcpy(value, line + splitIndex + 1, valueLen);
            value[strlen(line) - splitIndex - 1] = '\0';
            return 0;
        } else
            return -1;
    } else
        return -1;
}

long string_to_long(const string src) {
    string s = src;

    return strtol(s.c_str(), NULL, 0);
}

long string_to_long(const char *src) {
    return string_to_long((string) src);
}

unsigned long string_to_ulong(const string src) {
    string s = src;

    return strtoul(s.c_str(), NULL, 0);
}

unsigned long string_to_ulong(const char *src) {
    return string_to_ulong((string) src);
}

bool inside_int(int target, int min, int max) {
    return target >= min && target <= max;
}

bool inside_float(float target, float min, float max) {
    return target >= min && target <= max;
}

bool inside_long(long target, long min, long max) {
    return target >= min && target <= max;
}

void string_to_lower(char *src, int size) {
    int i = 0;

    while(src[i] && i < size) {
        src[i] = tolower(src[i]);
        i++;
    }
}

void string_to_upper(char *src, int size) {
    int i = 0;

    while(src[i] && i < size) {
        src[i] = toupper(src[i]);
        i++;
    }
}

void string_split(const string & src, const string & separator, list < string > &dest) {
    string str = src;
    string substring;

    string::size_type start = 0, index;

    do {
        index = str.find_first_of(separator, start);
        if(index != string::npos) {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator, index);
            if(start == string::npos)
                return;
        }
    } while(index != string::npos);

    //the last token
    substring = str.substr(start);
    dest.push_back(substring);
}
