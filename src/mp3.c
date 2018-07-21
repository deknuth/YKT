#include "../inc/main.h"
#define BUFSIZE 512
int sfd = -1;
struct buffer {
    FILE *fp;                       // file pointer
    unsigned int flen;              // file length
    unsigned int fpos;              // current position
    unsigned char fbuf[BUFSIZE];    // buffer
    unsigned int fbsize;            // indeed size of buffer
};
typedef struct buffer mp3_file;
unsigned int prerate = 0; // the pre simple rate
int soundfd;

int writedsp(int c)
{
    return write(soundfd, (char *)&c, 1);
}

void set_dsp(void)
{
#if 1
    int format = AFMT_S16_LE;
    int channels = 2;
    int rate = 4000;

    soundfd = open("/dev/dsp", O_WRONLY);
    if(soundfd < 0)
        lprintf(lfd,FATAL,"Audio: can not open /dev/dsp !");
    ioctl(soundfd, SNDCTL_DSP_SPEED,&rate);
    ioctl(soundfd, SNDCTL_DSP_SETFMT, &format);
    ioctl(soundfd, SNDCTL_DSP_CHANNELS, &channels);
#else
    /*&
    if((soundfd = open("test.bin" , O_WRONLY | O_CREAT)) < 0)
    {
        fprintf(stderr , "can't open sound device!\n");
        exit(-1);
    }
*/
#endif
}

static int decode(mp3_file *mp3fp);

void PlayAudio(void *arg)
{
    long flen, fsta, fend;
    int value = *(int *)arg;
    char name[128] = {0};
    if(sfd > 0)
        return;
    mp3_file *mp3fp = (mp3_file *)malloc(sizeof(mp3_file));
    switch(value)
    {
    case 0:
        snprintf(name,127,"%s","/home/root/YKT/audio/0.mp3");
        break;
    case 1:
        snprintf(name,127,"%s","/home/root/YKT/audio/1.mp3");
        break;
    case 2:
        snprintf(name,127,"%s","audio/2.mp3");
        break;
    case 3:
        snprintf(name,127,"%s","audio/3.mp3");
        break;
    case 4:
        snprintf(name,127,"%s","audio/4.mp3");
        break;
    case 5:
        snprintf(name,127,"%s","audio/5.mp3");
        break;
    case 6:
        snprintf(name,127,"%s","audio/6.mp3");
        break;
    default:
        snprintf(name,127,"%s","audio/0.mp3");
        break;
    }
    m_printf("mp3 file: %s\n",name);
    if((mp3fp->fp = fopen(name, "r")) == NULL)
    {
        lprintf(lfd,FATAL,"Audio: can not open audio file!");
        free(mp3fp);
        sfd = -1;
        return;
    }
    else
        sfd = 1;
    fsta = ftell(mp3fp->fp);
    fseek(mp3fp->fp, 0, SEEK_END);
    fend = ftell(mp3fp->fp);
    flen = fend - fsta;
    if(flen > 0)
        fseek(mp3fp->fp, 0, SEEK_SET);
    fread(mp3fp->fbuf, 1, BUFSIZE, mp3fp->fp);
    mp3fp->fbsize = BUFSIZE;
    mp3fp->fpos = BUFSIZE;
    mp3fp->flen = flen;

    set_dsp();

    decode(mp3fp);
    close(soundfd);
    fclose(mp3fp->fp);
    free(mp3fp);
    sfd = -1;
    return;
}

static enum mad_flow input(void *data, struct mad_stream *stream)
{
    mp3_file *mp3fp;
    int ret_code;
    int unproc_data_size;   // the unprocessed data's size
    int copy_size;
    size_t len = 0;
    mp3fp = (mp3_file *)data;
    if(mp3fp->fpos < mp3fp->flen)
    {
        unproc_data_size = stream->bufend - stream->next_frame;
        //printf("%d, %d, %d\n", unproc_data_size, mp3fp->fpos, mp3fp->fbsize);
        memcpy(mp3fp->fbuf, mp3fp->fbuf + mp3fp->fbsize - unproc_data_size, unproc_data_size);
        copy_size = BUFSIZE - unproc_data_size;
        if(mp3fp->fpos + copy_size > mp3fp->flen)
            copy_size = mp3fp->flen - mp3fp->fpos;
        len = fread(mp3fp->fbuf+unproc_data_size, 1, copy_size, mp3fp->fp);
        mp3fp->fbsize = unproc_data_size + copy_size;
        mp3fp->fpos += copy_size;
        /*Hand off the buffer to the mp3 input stream*/
        mad_stream_buffer(stream, mp3fp->fbuf, mp3fp->fbsize);
        ret_code = MAD_FLOW_CONTINUE;
    }
    else
        ret_code = MAD_FLOW_STOP;
    return ret_code;
}


static inline signed int scale(mad_fixed_t sample)
{
    sample += (1L << (MAD_F_FRACBITS - 16));
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}


//输出函数做相应的修改，目的是解决播放音乐时声音卡的问题。
static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples;
    mad_fixed_t const *left_ch, *right_ch;
    // pcm->samplerate contains the sampling frequency
    nchannels = pcm->channels;
    nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];
    short buf[nsamples *2];
    int i = 0;
    //printf(">>%d\n", nsamples);
    while (nsamples--)
    {
        signed int sample;
        // output sample(s) in 16-bit signed little-endian PCM
        sample = scale(*left_ch++);
        buf[i++] = sample & 0xFFFF;
        if (nchannels == 2)
        {
            sample = scale(*right_ch++);
            buf[i++] = sample & 0xFFFF;
        }
    }
    write(soundfd, &buf[0], i * 2);
    return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
    mp3_file *mp3fp = data;
//    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",stream->error, mad_stream_errorstr(stream),stream->this_frame - mp3fp->fbuf);
    return MAD_FLOW_CONTINUE;
}

static int decode(mp3_file *mp3fp)
{
    struct mad_decoder decoder;
    int result;

    mad_decoder_init(&decoder, mp3fp,
                     input, 0 /* header */, 0 /* filter */, output,
                     error, 0 /* message */);
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    mad_decoder_finish(&decoder);
    return result;
}

/*
struct buffer {
    unsigned char const *start;
    unsigned long length;
};
static int sfd = 0;
static int decode(unsigned char const *, unsigned long);

// 每个回调函数会返回一个枚举类型mad_flow,通过mad_flow可以控制解码的过程
// 应该input，output各一个mad_flow回调函数

// 这个input函数表示将数据全部读进缓冲区，之后将缓冲区长度置0
static enum mad_flow input(void *data, struct mad_stream *stream)
{
    struct buffer *buffer = data;
    if (!buffer->length)
        return MAD_FLOW_STOP;
    //把原始的未解码的 MPEG 数据和 mad_stream 数据结构关联，以便使用 mad_frame_decode( ) 来解码 MPEG 帧数据
    mad_stream_buffer(stream, buffer->start, buffer->length);
    buffer->length = 0;
    return MAD_FLOW_CONTINUE;
}


static inline signed int scale(mad_fixed_t sample)
{
    sample += (1L << (MAD_F_FRACBITS - 16));
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples, n;
    mad_fixed_t const *left_ch, *right_ch;
    unsigned char Output[8192], *OutputPtr;
    int wrote;
    int fmt, speed;
    int stere = 0;
    int sample = 16;
    nchannels = pcm->channels;
    n = nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];

    fmt = AFMT_S16_LE;
    speed = pcm->samplerate << 1;    //播放速度为采样率两倍
    printf("speed: %d\n",speed);
    printf("pcm->channels: %d\n",pcm->length);

 //   ioctl(sfd, SNDCTL_DSP_SPEED, &speed);
  //  ioctl(sfd, SNDCTL_DSP_SETFMT, &fmt);
  //  ioctl(sfd, SNDCTL_DSP_CHANNELS, &(pcm->channels));
    //   ioctl(sfd, SNDCTL_DSP_STEREO, &stere);
    //  ioctl(sfd, SNDCTL_DSP_SAMPLESIZE, &sample);

    OutputPtr = Output;
    while (nsamples--)
    {
        signed int sample;
        sample = scale(*left_ch++);
        *(OutputPtr++) = sample >> 0;
        *(OutputPtr++) = sample >> 8;
        if (nchannels == 2)
        {
            sample = scale(*right_ch++);
            *(OutputPtr++) = sample >> 0;
            *(OutputPtr++) = sample >> 8;
        }
    }
    n <<= 2;         //数据长度为pcm音频的4倍
    OutputPtr = Output;
    while (n)
    {
        wrote = write(sfd, OutputPtr, n);
        OutputPtr += wrote;
        n -= wrote;
    }
    OutputPtr = Output;

    return MAD_FLOW_CONTINUE;
}


static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
    return MAD_FLOW_CONTINUE;
}

static int decode(unsigned char const *start, unsigned long length)
{
    struct buffer buffer;  //上面定义的buffer，内有内存起点和大小
    struct mad_decoder decoder;   //struct mad_decoder 定义解码器结构体
    int result;
    buffer.start = start;
    buffer.length = length; // 传送给结构体

    //mad_decoder_init 初始化函数
    // 参数如下，decoder,buffer,input,output,error
    mad_decoder_init(&decoder, &buffer, input, 0, 0, output, error, 0);
    mad_decoder_options(&decoder, 0);
    // mad_decoder_run解码函数，解码过程中output函数被不断调用
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    mad_decoder_finish(&decoder);
    return result;
}

void SetDsp(int fd)
{
    int format = AFMT_S16_LE;
    int channels = 1;
    int rate = 16000;

    ioctl(fd, SNDCTL_DSP_SPEED,&rate);
    ioctl(fd, SNDCTL_DSP_SETFMT, &format);
    ioctl(fd, SNDCTL_DSP_CHANNELS, &channels);
}

int PlayAudio(int arg)
{
    if(sfd > 0)
        return 6;
    struct stat stat;
    void *fdm;
    int fd;
    char name[128] = {0};
    switch(arg)
    {
    case 0:
        snprintf(name,127,"%s","audio/0.mp3");
        break;
    case 1:
        snprintf(name,127,"%s","audio/1.mp3");
        break;
    case 2:
        snprintf(name,127,"%s","audio/2.mp3");
        break;
    case 3:
        snprintf(name,127,"%s","audio/3.mp3");
        break;
    default:
        snprintf(name,127,"%s","audio/3.mp3");
        break;
    }
    printf("mp3 file: %s\n",name);
    if((fd = open(name, O_RDONLY)) == -1)
    {
        lprintf(lfd,FATAL,"Audio: can not open audio file!");
        return 1;
    }

    if ((sfd = open("/dev/dsp", O_WRONLY)) < 0)
    {
        lprintf(lfd,FATAL,"Audio: can not open audio device!");
        return 5;
    }

    ioctl(sfd, SNDCTL_DSP_SYNC, 0); //控制端口，
    if (fstat(fd, &stat) == -1 || stat.st_size == 0)
    {
        lprintf(lfd,FATAL,"Audio: stat failed!");
        sfd = -1;
        return 2;
    }
    //mmap内存映射函数，将音频文件映射至fdm（映射区域的指针）
    fdm = mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fdm == MAP_FAILED)
    {
        lprintf(lfd,FATAL,"Audio: mmap failed!");
        sfd = -1;
        return 3;
    }
    SetDsp(sfd);
    decode(fdm, stat.st_size);

    if (munmap(fdm, stat.st_size) == -1)
    {
        lprintf(lfd,FATAL,"Audio: munmap failed!");
        sfd = -1;
        return 4;
    }

    ioctl(sfd, SNDCTL_DSP_RESET, 0);
    close(sfd);
    sfd = -1;
    return 0;
}
*/
