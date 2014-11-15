#ifndef PAMON_H
#define PAMON_H
/**
 * Originally from
 * GPL 3 Ian Halpern https://github.com/ianhalpern/Impulse
 */
#include <pulse/pulseaudio.h>
#include <stdlib.h>
#include <fftw3.h>

static pa_stream *stream = NULL;
static pa_context *context = NULL;
static pa_io_event *stdio_event = NULL;
static pa_mainloop_api *mainloop_api = NULL;
static pa_threaded_mainloop *mainloop = NULL;
static char *stream_name = NULL, *client_name = NULL, *device = NULL;

static pa_stream_flags_t flags = 0;

#define CHUNK 4096
#define MAX_SAMPLES (CHUNK/12) // Gives out to about 15khz
// 15,000 / (4096/12) = ~43hz per bar
static GLfloat magnitudeFFT[MAX_SAMPLES];
static GLfloat magnitudeRaw[CHUNK/4];
static GLfloat fft_max[MAX_SAMPLES] = {22229550.000000, 31989850.000000, 28597552.000000, 19983404.000000, 16425934.000000, 16403634.000000, 16398000.000000, 16391887.000000, 16390757.000000, 16387198.000000, 16384857.000000, 16381337.000000, 16384348.000000, 16383889.000000, 16380952.000000, 16382161.000000, 16375936.000000, 16373104.000000, 16377100.000000, 16376355.000000, 16372127.000000, 16377310.000000, 16373027.000000, 16368166.000000, 16372622.000000, 16369351.000000, 16370629.000000, 16364831.000000, 16368793.000000, 16363286.000000, 16370294.000000, 16372832.000000, 16362928.000000, 16364312.000000, 16350736.000000, 16348429.000000, 16345807.000000, 16349836.000000, 16340731.000000, 16360303.000000, 16348070.000000, 16345882.000000, 16340568.000000, 16342663.000000, 16338285.000000, 16335694.000000, 16314398.000000, 16319549.000000, 16340597.000000, 16321103.000000, 16314123.000000, 16324755.000000, 16314840.000000, 16325576.000000, 16325509.000000, 16284793.000000, 16321548.000000, 16287778.000000, 16287048.000000, 16305562.000000, 16309619.000000, 16301458.000000, 16304921.000000, 16304476.000000, 16292239.000000, 16303065.000000, 16280965.000000, 16280469.000000, 16283727.000000, 16288066.000000, 16282537.000000, 16274914.000000, 16259085.000000, 16273397.000000, 16234719.000000, 16270275.000000, 16256148.000000, 16253431.000000, 16259134.000000, 16261040.000000, 16237265.000000, 16229602.000000, 16251743.000000, 16226971.000000, 16186049.000000, 16224581.000000, 16239414.000000, 16220225.000000, 16217086.000000, 16219406.000000, 16164796.000000, 16223425.000000, 16174499.000000, 16210614.000000, 16213731.000000, 16209536.000000, 16180941.000000, 16109510.000000, 16134499.000000, 16114933.000000, 16097555.000000, 16144815.000000, 16090357.000000, 16163671.000000, 16163998.000000, 16061792.000000, 16156961.000000, 16062236.000000, 16155536.000000, 16051141.000000, 16127134.000000, 16135170.000000, 16089985.000000, 16028997.000000, 16006264.000000, 16012947.000000, 16093763.000000, 16073720.000000, 16018290.000000, 16037079.000000, 16046854.000000, 16095784.000000, 16077231.000000, 15993265.000000, 16063830.000000, 15973213.000000, 16068384.000000, 15943959.000000, 16052229.000000, 16024243.000000, 15915780.000000, 16051779.000000, 16035486.000000, 15960589.000000, 15979942.000000, 16024456.000000, 16003163.000000, 15986991.000000, 15973500.000000, 15977708.000000, 15980379.000000, 15988615.000000, 15975394.000000, 15974627.000000, 15945880.000000, 15889121.000000, 15832182.000000, 15959782.000000, 15935185.000000, 15773231.000000, 15941364.000000, 15852114.000000, 15890071.000000, 15889857.000000, 15860585.000000, 15911205.000000, 15752317.000000, 15879423.000000, 15859516.000000, 15866490.000000, 15877144.000000, 15735754.000000, 15809971.000000, 15843647.000000, 15852571.000000, 15772044.000000, 15636022.000000, 15810937.000000, 15820450.000000, 15811923.000000, 15800377.000000, 15814729.000000, 15794159.000000, 15785822.000000, 15781943.000000, 15759591.000000, 15756076.000000, 15773811.000000, 15733243.000000, 15754423.000000, 15688364.000000, 15524243.000000, 15577869.000000, 15705724.000000, 15715442.000000, 15689333.000000, 15649291.000000, 15691424.000000, 15582906.000000, 15648896.000000, 15665001.000000, 15640970.000000, 15560833.000000, 15466380.000000, 15638976.000000, 15329427.000000, 15602938.000000, 15331012.000000, 15600536.000000, 15266013.000000, 15586584.000000, 15372327.000000, 15570875.000000, 15507834.000000, 15421949.000000, 15551447.000000, 15331494.000000, 15528355.000000, 15534993.000000, 15192585.000000, 15444993.000000, 15505091.000000, 15361548.000000, 15158376.000000, 15434503.000000, 15457709.000000, 15418287.000000, 15185052.000000, 15175857.000000, 15363960.000000, 15439526.000000, 15420590.000000, 15311991.000000, 14823466.000000, 15019476.000000, 15164304.000000, 15238122.000000, 15295085.000000, 15297523.000000, 15321598.000000, 15310042.000000, 15305253.000000, 15284303.000000, 15239164.000000, 15199130.000000, 15104577.000000, 14997816.000000, 14826093.000000, 14807165.000000, 15077583.000000, 14804491.000000, 15033129.000000, 15179526.000000, 15221548.000000, 15203570.000000, 15048715.000000, 15177531.000000, 14949038.000000, 15158644.000000, 15077937.000000, 15148647.000000, 15045164.000000, 15098798.000000, 15046945.000000, 15110640.000000, 14891903.000000, 15113893.000000, 15099020.000000, 14824189.000000, 14800292.000000, 15049485.000000, 14755327.000000, 14844738.000000, 15032947.000000, 14468810.000000, 14980449.000000, 14873277.000000, 14677205.000000, 14973139.000000, 14267255.000000, 14952213.000000, 14538616.000000, 14859452.000000, 14680780.000000, 14790715.000000, 14720747.000000, 14354817.000000, 14880593.000000, 14248893.000000, 14863950.000000, 14403978.000000, 14834459.000000, 14430588.000000, 14716644.000000, 14750329.000000, 14448217.000000, 14792249.000000, 14097398.000000, 14741174.000000, 14589825.000000, 14406729.000000, 14724860.000000, 14313893.000000, 14575146.000000, 14710156.000000, 14116005.000000, 14674742.000000, 14031248.000000, 14405720.000000, 14654505.000000, 14302642.000000, 14148464.000000, 14622552.000000, 14494339.000000, 13784703.000000, 14382702.000000, 14556947.000000, 14397776.000000, 13716456.000000, 14318955.000000, 14510176.000000, 14454840.000000, 13994300.000000, 14011821.000000, 14418538.000000, 14432590.000000, 14351621.000000, 13920161.000000, 14205056.000000, 14418558.000000, 14358237.000000, 14039499.000000, 13475913.000000, 13843258.000000, 14170703.000000, 14321193.000000, 14305003.000000, 14289330.000000, 14174175.000000, 13926454.000000, 13599834.000000, 13303752.000000, 13643680.000000, 13845225.000000, 14007142.000000, 14126939.000000, 14175856.000000, 14179615.000000, 14152679.000000, 14132522.000000, 14137483.000000};

static int16_t buffer[CHUNK/2], snapshot[CHUNK/2];
static size_t buffer_index = 0;

static pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
};

static pa_channel_map channel_map;
static int channel_map_set = 0;

static void quit_pamon(int ret)
{
    assert(mainloop_api);
    mainloop_api->quit(mainloop_api, ret);
}

static void stream_read_callback(pa_stream *s, size_t length, void *userdata)
{
    const void *data;
    assert(s);
    assert(length>0);

    if (stdio_event)
        mainloop_api->io_enable(stdio_event, PA_IO_EVENT_OUTPUT);

    if (pa_stream_peek(s, &data, &length) < 0) {
        fprintf(stderr, "pa_stream_peek() failed: %s\n", pa_strerror(pa_context_errno(context)));
        quit_pamon(1);
        return;
    }

    assert(data);
    assert(length>0);

    int excess = buffer_index * 2 + length - CHUNK;

    if (excess < 0)
        excess = 0;

    memcpy(buffer+buffer_index, data, length-excess);
    buffer_index += (length-excess)/2;

    if(excess) {
        memcpy(snapshot, buffer, buffer_index*2);
        buffer_index = 0;
    }

    pa_stream_drop(s);
}

static void get_source_info_callback( pa_context *c, const pa_source_info *i, int is_last, void *userdata ) {
    if ( is_last || device != NULL )
        return;
    assert(i);
    // snprintf(t, sizeof(t), "%u", i->monitor_of_sink);
    if ( i->monitor_of_sink != PA_INVALID_INDEX ) {
        device = pa_xstrdup( i->name );
        if ((pa_stream_connect_record( stream, device, NULL, flags)) < 0 ) {
            fprintf(stderr, "pa_stream_connect_record() failed: %s\n", pa_strerror(pa_context_errno(c)));
            quit_pamon(1);
        }
    }
}

static void context_state_callback(pa_context *c, void *userdata)
{
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY:
            assert(c);
            assert(!stream);
            if (!(stream = pa_stream_new(c, stream_name, &sample_spec, channel_map_set ? &channel_map : NULL))) {
                fprintf(stderr, "pa_stream_new() failed: %s\n", pa_strerror(pa_context_errno(c)));
                quit_pamon(1);
            }
            pa_stream_set_read_callback(stream, stream_read_callback, NULL);
            pa_operation_unref( pa_context_get_source_info_list(c, get_source_info_callback, NULL ));
            break;
        case PA_CONTEXT_TERMINATED:
            quit_pamon(0);
            break;
        case PA_CONTEXT_FAILED:
        default:
            fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
            quit_pamon(1);
    }
}

static inline
void im_getSnapshot()
{
    int i, j;
    double *in;
    fftw_complex *out;
    fftw_plan p;
    in = malloc(sizeof(double)*(CHUNK/2));
    assert(in!=NULL);
    out = fftw_malloc(sizeof(fftw_complex)*(CHUNK / 2));
    assert(out!=NULL);
    for (i = 0; i < CHUNK / 2; i++) {
        in[i] = (double) snapshot[i];
    }
    p = fftw_plan_dft_r2c_1d(CHUNK/2, in, out, FFTW_PATIENT);
    fftw_execute(p);
    fftw_destroy_plan(p);

    for (i = 0; i < CHUNK / 2; i += sample_spec.channels) {
        magnitudeRaw[i / sample_spec.channels] = 0;
        for (j = 0; j < sample_spec.channels; j++) {
            if (i+j < MAX_SAMPLES) {
                // fill FFT, combined channels
                magnitudeFFT[i+j] = (GLfloat)((GLfloat)sqrt(pow(out[i+j][0], 2) + pow(out[i+j][1], 2))/(GLfloat) fft_max[i+j]);
                if (magnitudeFFT[i+j] > 1.0) {
                    fft_max[i+j] = fft_max[i+j]*0.9 + 0.1*(GLfloat)(sqrt(pow(out[i+j][0], 2) + pow(out[i+j][1], 2)));
                    magnitudeFFT[i+j] = 1.0;
                }
            }
            // fill RAW data, combined channels
            magnitudeRaw[i/sample_spec.channels] += ((GLfloat)snapshot[i+j]/(pow(2, 16)/2))/sample_spec.channels;
        }
    }

    free(in);
    fftw_free(out);
}

#define CONFIG_FILE "pamon.conf"
static inline
void load_pamon_config(void)
{
    SDL_RWops *f = SDL_RWFromFile(CONFIG_FILE, "rb");
    if (!f) {
        fprintf(stderr, "Warning: could not open config file\n");
        return;
    }
    GLint num_samples;
    if (f->read(f, &num_samples, sizeof(GLint), 1) != 1) {
        fprintf(stderr, "Error: Failed reading file\n");
        goto END;
    }
    if (num_samples != MAX_SAMPLES) {
        fprintf(stderr, "Error: Incorrect number of samples in config\n");
        goto END;
    }
    if (f->read(f, &fft_max, sizeof(GLfloat), num_samples) != num_samples) {
        fprintf(stderr, "Error: Failed reading file\n");
        goto END;
    }

    printf("successfully loaded config\n");
END:
    if(f)
        f->close(f);
}

static inline
void save_pamon_config(void)
{
    SDL_RWops *f = SDL_RWFromFile(CONFIG_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Warning: could not open config file\n");
        return;
    }
    GLint tmp = MAX_SAMPLES;
    if (f->write(f, &tmp, sizeof(GLint), 1) != 1) {
        fprintf(stderr, "Error: could not write number to config\n");
        goto END;
    }
    if (f->write(f, &fft_max, sizeof(GLfloat), MAX_SAMPLES) != MAX_SAMPLES) {
        fprintf(stderr, "Error: could not write values to config\n");
        goto END;
    }
    printf("Config saved!\n");
END:
    if(f)
        f->close(f);
}

static inline
void initPAMon()
{
    int r;
    char *server = NULL;
    client_name = pa_xstrdup("visualr");
    stream_name = pa_xstrdup("visualr");
    if (!(mainloop = pa_threaded_mainloop_new())) {
        fprintf(stderr, "pa_threaded_mainloop_new failed\n");
        return;
    }

    // int i;
    // for (i=0; i<MAX_SAMPLES;i++)
    //     fft_max[i] = 0.1;

    mainloop_api = pa_threaded_mainloop_get_api(mainloop);
    r = pa_signal_init(mainloop_api);
    assert(r==0);

    if (!(context = pa_context_new(mainloop_api, client_name))) {
        fprintf(stderr, "pa_context_new() failed \n");
        return;
    }

    pa_context_set_state_callback(context, context_state_callback, NULL);
    pa_context_connect(context, server, 0, NULL);
    pa_threaded_mainloop_start(mainloop);

    printf("Creating FFT plan..\n");
    im_getSnapshot();
}
#endif
