# BugBench

## Status

### Original BugBench

| ID    | Program            | Sources | Build | Reproduction |
|-------|--------------------|---------|-------|--------------|
| NCOM  | `ncompress-4.2.4`  | ✓       | ✓     | ✓            |
| POLY  | `polymorph-0.4.0`  | ✓       | ✓     | ✓            |
| GZIP  | `gzip-1.2.4`       | ✓       | ✓     | ✓            |
| MAN   | `man-1.5h1`        |         |       |              |
| GO    | `099.go`           |         |       |              |
| COMP  | `129.compress`     |         |       |              |
| BC    | `bc-1.06`          | ✓       | ✓     | ✓            |
| SQUD  | `squid-2.3`        | ✓       | ✓     |              |
| CALB  | `cachelib-?`       |         |       |              |
| CVS   | `cvs-1.11.4`       | ✓       | ✓     |              |
| YPSV  | `ypserv-2.2`       | ✓       | ✓     |              |
| PFTP  | `proftpd-1.2.9`    |         |       |              |
| SQUD2 | `squid-2.4`        |         |       |              |
| HTPD1 | `httpd-2.0.49`     | ✓       |       |              |
| MSQL1 | `msql-4.1.1`       | ✓       |       |              |
| MSQL2 | `msql-3.23.56`     | ✓       |       |              |
| MSQL3 | `msql-4.1.1`       | ✓       |       |              |
| PSQL  | `postgresql-7.4.2` |         |       |              |
| HTPD2 | `httpd2.0.49`      | ✓       |       |              |

### BugsCpp

| ID    | Program             | Sources | Build | Reproduction | Information    |
|-------|---------------------|---------|-------|--------------|----------------|
| PEG   | `cpp-peglib-0.1.12` | ✓       | ✓     | ✓            | CVE-2020-23914 |
| YAML1 | `yaml-cpp-0.6.3`    | ✓       | ✓     | ✓            | CVE-2018-20573 |
| YAML2 | `yaml-cpp-0.6.2`    | ✓       | ✓     | ✓            | CVE-2017-11692 |

## Notes

### POLY

Changes were needed to the source code to fix some build errors, see
[7fc9fca](https://github.com/nicovank/bugbench/commit/7fc9fca339234d825496e23792f04be41744fa06).

### BC

Input obtained from Shan Lu.

### SQUD

Full version is `2.3.STABLE5`.

Changes were needed to the source code to fix a build error, see
[21be15e](https://github.com/nicovank/bugbench/commit/21be15e7c915df46412bd881ff2eb04ce8c78858).

### CVS

To avoid changing source code, CVS must be built in 32 bit mode.

### YPSV

Changes were needed to the build infrastructure, see
[56bce62](https://github.com/nicovank/bugbench/commit/56bce62f28f60f1e3f432ec5ced58194938bee28).
Source code is unchanged.

### PEG

Catch2 needed updating to fix a compilation error, see
[4b00d31](https://github.com/nicovank/bugbench/commit/4b00d311656bed2b3b8f7a8858e782aa3d168074).
