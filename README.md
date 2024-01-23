# BugBench

## Status

| ID    | Program            | Sources | Build | Reproduction |
|-------|--------------------|---------|-------|--------------|
| NCOM  | `ncompress-4.2.4`  | ✅      | ✅    | ✅           |
| POLY  | `polymorph-0.4.0`  | ✅      |       |              |
| GZIP  | `gzip-1.2.4`       | ✅      | ✅    | ✅           |
| MAN   | `man-1.5h1`        |         |       |              |
| GO    | `099.go`           |         |       |              |
| COMP  | `129.compress`     |         |       |              |
| BC    | `bc-1.06`          | ✅      | ✅    | ✅           |
| SQUD  | `squid-2.3`        |         |       |              |
| CALB  | `cachelib-?`       |         |       |              |
| CVS   | `cvs-1.11.4`       | ✅      | ✅    |              |
| YPSV  | `ypserv-2.2`       | ✅      | ✅    |              |
| PFTP  | `proftpd-1.2.9`    |         |       |              |
| SQUD2 | `squid-2.4`        |         |       |              |
| HTPD1 | `httpd-2.0.49`     | ✅      |       |              |
| MSQL1 | `msql-4.1.1`       | ✅      |       |              |
| MSQL2 | `msql-3.23.56`     | ✅      |       |              |
| MSQL3 | `msql-4.1.1`       | ✅      |       |              |
| PSQL  | `postgresql-7.4.2` |         |       |              |
| HTPD2 | `httpd2.0.49`      | ✅      |       |              |

## Notes

### BC

Input obtained from Shan Lu.

### CVS

To avoid changing source code, CVS must be built in 32 bit mode.

### YPSV

Changes were needed to the build infrastructure, see
[56bce62](https://github.com/nicovank/bugbench/commit/56bce62f28f60f1e3f432ec5ced58194938bee28).
Source code is unchanged.

