#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <netcdf.h>
#include <sys/stat.h>

#define NC_CHECK(call) do { \
    int status = (call); \
    if (status != NC_NOERR) { \
        fprintf(stderr, "NetCDF error: %s\n", nc_strerror(status)); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define START_YR 1991
#define END_YR   2020
#define DELTA_DAY 5
#define ROW_TOTAL 721
#define COL_TOTAL 1440
#define SAMPLE_TOTAL 330

const char *nc_path = "/public/home/achwjznh4b/Newdata/";
const char *save_path = "/public/home/mcc20262029/lyh/ERA5/Climatology/";

const char *var_sst = "data";
const char *var_lon = "lon";
const char *var_lat = "lat";

int is_leap(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* 把无闰年的 day-of-year 转成月日 */
void doy_to_month_day(int year, int doy, int *month, int *day) {
    int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    for (int m = 0; m < 12; m++) {
        if (doy <= mdays[m]) {
            *month = m + 1;
            *day = doy;
            return;
        }
        doy -= mdays[m];
    }
}

/* 简化版：窗口前后 5 天，不跨年处理 */
void make_date_string(int year, int doy, char *out) {
    int month, day;
    doy_to_month_day(year, doy, &month, &day);
    sprintf(out, "%04d%02d%02d", year, month, day);
}

int cmp_double(const void *a, const void *b) {
    double x = *(double *)a;
    double y = *(double *)b;
    return (x > y) - (x < y);
}

double nanmean(double *arr, int n) {
    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (!isnan(arr[i])) {
            sum += arr[i];
            count++;
        }
    }

    if (count == 0) return NAN;
    return sum / count;
}

double percentile90(double *arr, int n) {
    double tmp[SAMPLE_TOTAL];
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (!isnan(arr[i])) {
            tmp[count++] = arr[i];
        }
    }

    if (count == 0) return NAN;

    qsort(tmp, count, sizeof(double), cmp_double);

    double pos = 0.9 * (count - 1);
    int low = (int)floor(pos);
    int high = (int)ceil(pos);
    double frac = pos - low;

    if (low == high) return tmp[low];
    return tmp[low] * (1.0 - frac) + tmp[high] * frac;
}

void read_lon_lat(double **lon, double **lat, int *lon_num, int *lat_num) {
    char demo_file[512];
    sprintf(demo_file, "%s19910101", nc_path);

    if (!file_exists(demo_file)) {
        fprintf(stderr, "Demo file does not exist: %s\n", demo_file);
        exit(EXIT_FAILURE);
    }

    int ncid, lon_varid, lat_varid;
    int lon_dimid, lat_dimid;
    size_t lon_len, lat_len;

    NC_CHECK(nc_open(demo_file, NC_NOWRITE, &ncid));
    NC_CHECK(nc_inq_varid(ncid, var_lon, &lon_varid));
    NC_CHECK(nc_inq_varid(ncid, var_lat, &lat_varid));

    NC_CHECK(nc_inq_dimid(ncid, "lon", &lon_dimid));
    NC_CHECK(nc_inq_dimid(ncid, "lat", &lat_dimid));
    NC_CHECK(nc_inq_dimlen(ncid, lon_dimid, &lon_len));
    NC_CHECK(nc_inq_dimlen(ncid, lat_dimid, &lat_len));

    *lon_num = (int)lon_len;
    *lat_num = (int)lat_len;

    *lon = malloc(lon_len * sizeof(double));
    *lat = malloc(lat_len * sizeof(double));

    NC_CHECK(nc_get_var_double(ncid, lon_varid, *lon));
    NC_CHECK(nc_get_var_double(ncid, lat_varid, *lat));

    NC_CHECK(nc_close(ncid));
}

void write_output_nc(
    const char *filename,
    double *lon,
    double *lat,
    int lon_num,
    int lat_num,
    int doy,
    double *Clim,
    double *P90
) {
    int ncid;
    int lat_dimid, lon_dimid, time_dimid;
    int lat_varid, lon_varid, time_varid;
    int clim_varid, p90_varid;

    if (file_exists(filename)) {
        remove(filename);
    }

    NC_CHECK(nc_create(filename, NC_NETCDF4, &ncid));

    NC_CHECK(nc_def_dim(ncid, "Lat", lat_num, &lat_dimid));
    NC_CHECK(nc_def_dim(ncid, "Lon", lon_num, &lon_dimid));
    NC_CHECK(nc_def_dim(ncid, "Day", 1, &time_dimid));

    NC_CHECK(nc_def_var(ncid, "dayofyear", NC_DOUBLE, 1, &time_dimid, &time_varid));
    NC_CHECK(nc_put_att_text(ncid, time_varid, "long_name",
                             strlen("Day of year (1-365, no 29Feb)"),
                             "Day of year (1-365, no 29Feb)"));

    NC_CHECK(nc_def_var(ncid, "Lat", NC_DOUBLE, 1, &lat_dimid, &lat_varid));
    NC_CHECK(nc_def_var(ncid, "Lon", NC_DOUBLE, 1, &lon_dimid, &lon_varid));

    int dims[2] = {lat_dimid, lon_dimid};

    NC_CHECK(nc_def_var(ncid, "Climmean", NC_DOUBLE, 2, dims, &clim_varid));
    NC_CHECK(nc_put_att_text(ncid, clim_varid, "long_name",
                             strlen("OSTIA SST climatology 1991-2020"),
                             "OSTIA SST climatology 1991-2020"));

    NC_CHECK(nc_def_var(ncid, "P90_sst", NC_DOUBLE, 2, dims, &p90_varid));
    NC_CHECK(nc_put_att_text(ncid, p90_varid, "long_name",
                             strlen("90th percentile of SST"),
                             "90th percentile of SST"));

    NC_CHECK(nc_enddef(ncid));

    double doy_double = (double)doy;

    NC_CHECK(nc_put_var_double(ncid, lat_varid, lat));
    NC_CHECK(nc_put_var_double(ncid, lon_varid, lon));
    NC_CHECK(nc_put_var_double(ncid, time_varid, &doy_double));
    NC_CHECK(nc_put_var_double(ncid, clim_varid, Clim));
    NC_CHECK(nc_put_var_double(ncid, p90_varid, P90));

    NC_CHECK(nc_close(ncid));
}

int main() {
    double *lon = NULL;
    double *lat = NULL;
    int lon_num, lat_num;

    read_lon_lat(&lon, &lat, &lon_num, &lat_num);

    printf("lon_num = %d, lat_num = %d\n", lon_num, lat_num);

    double *Clim = malloc(lat_num * lon_num * sizeof(double));
    double *P90  = malloc(lat_num * lon_num * sizeof(double));
    double *sst_temp = malloc(SAMPLE_TOTAL * lon_num * sizeof(double));
    double *sst_row = malloc(lon_num * sizeof(double));
    double sample_values[SAMPLE_TOTAL];

    for (int doy = 152; doy <= 243; doy++) {
        printf("Calculating climatology for day %d...\n", doy);

        for (int i = 0; i < lat_num * lon_num; i++) {
            Clim[i] = NAN;
            P90[i] = NAN;
        }

        for (int row = 0; row < ROW_TOTAL; row++) {
            for (int i = 0; i < SAMPLE_TOTAL * lon_num; i++) {
                sst_temp[i] = NAN;
            }

            int temp_idx = 0;

            for (int yr = START_YR; yr <= END_YR; yr++) {
                for (int offset = -DELTA_DAY; offset <= DELTA_DAY; offset++) {
                    int target_doy = doy + offset;

                    if (target_doy < 1 || target_doy > 365) {
                        temp_idx++;
                        continue;
                    }

                    char date_str[16];
                    char nc_file[512];

                    make_date_string(yr, target_doy, date_str);
                    sprintf(nc_file, "%s%s", nc_path, date_str);

                    if (!file_exists(nc_file)) {
                        printf("Warning: %s does not exist, skip.\n", nc_file);
                        temp_idx++;
                        continue;
                    }

                    int ncid, sst_varid;
                    size_t start[2] = {(size_t)row, 0};
                    size_t count[2] = {1, (size_t)lon_num};

                    NC_CHECK(nc_open(nc_file, NC_NOWRITE, &ncid));
                    NC_CHECK(nc_inq_varid(ncid, var_sst, &sst_varid));
                    NC_CHECK(nc_get_vara_double(ncid, sst_varid, start, count, sst_row));
                    NC_CHECK(nc_close(ncid));

                    for (int j = 0; j < lon_num; j++) {
                        sst_temp[temp_idx * lon_num + j] = sst_row[j];
                    }

                    temp_idx++;
                }
            }

            for (int j = 0; j < lon_num; j++) {
                for (int k = 0; k < SAMPLE_TOTAL; k++) {
                    sample_values[k] = sst_temp[k * lon_num + j];
                }

                Clim[row * lon_num + j] = nanmean(sample_values, SAMPLE_TOTAL);
                P90[row * lon_num + j] = percentile90(sample_values, SAMPLE_TOTAL);
            }

            printf("Finished %d/%d\n", row + 1, ROW_TOTAL);
        }

        int month, day;
        doy_to_month_day(2020, doy, &month, &day);

        char out_file[512];
        sprintf(out_file, "%s%02d%02d.nc", save_path, month, day);

        write_output_nc(out_file, lon, lat, lon_num, lat_num, doy, Clim, P90);

        printf("%s was created successfully.\n", out_file);
    }

    free(lon);
    free(lat);
    free(Clim);
    free(P90);
    free(sst_temp);
    free(sst_row);

    return 0;
}