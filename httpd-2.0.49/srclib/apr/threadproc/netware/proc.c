/* Copyright 2000-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr_arch_threadproc.h"
#include "apr_arch_file_io.h"
#include "apr_strings.h"
#include "apr_portable.h"

#include <proc.h>

apr_status_t apr_netware_proc_cleanup(void *theproc)
{
    apr_proc_t *proc = theproc;
    int exit_int;
    int waitpid_options = WUNTRACED | WNOHANG;

    if (proc->pid > 0) {
        waitpid(proc->pid, &exit_int, waitpid_options);
    }

/*	NXVmDestroy(proc->pid); */
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_create(apr_procattr_t **new,apr_pool_t *pool)
{
    (*new) = (apr_procattr_t *)apr_pcalloc(pool, sizeof(apr_procattr_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }
    (*new)->pool = pool;
    (*new)->cmdtype = APR_PROGRAM;
    /* Default to a current path since NetWare doesn't handle it very well */
    apr_filepath_get(&((*new)->currdir), APR_FILEPATH_NATIVE, pool);
    (*new)->detached = 1;
    return APR_SUCCESS;

}

APR_DECLARE(apr_status_t) apr_procattr_io_set(apr_procattr_t *attr, apr_int32_t in, 
                                 apr_int32_t out, apr_int32_t err)
{
    apr_status_t status;
    if (in != 0) {
        if ((status = apr_file_pipe_create(&attr->child_in, &attr->parent_in, 
                                   attr->pool)) != APR_SUCCESS) {
            return status;
        }
        switch (in) {
        case APR_FULL_BLOCK:
            break;
        case APR_PARENT_BLOCK:
            apr_file_pipe_timeout_set(attr->child_in, 0);
            break;
        case APR_CHILD_BLOCK:
            apr_file_pipe_timeout_set(attr->parent_in, 0);
            break;
        default:
            apr_file_pipe_timeout_set(attr->child_in, 0);
            apr_file_pipe_timeout_set(attr->parent_in, 0);
        }
    } 
    if (out) {
        if ((status = apr_file_pipe_create(&attr->parent_out, &attr->child_out, 
                                   attr->pool)) != APR_SUCCESS) {
            return status;
        }
        switch (out) {
        case APR_FULL_BLOCK:
            break;
        case APR_PARENT_BLOCK:
            apr_file_pipe_timeout_set(attr->child_out, 0);
            break;
        case APR_CHILD_BLOCK:
            apr_file_pipe_timeout_set(attr->parent_out, 0);
            break;
        default:
            apr_file_pipe_timeout_set(attr->child_out, 0);
            apr_file_pipe_timeout_set(attr->parent_out, 0);
        }
    } 
    if (err) {
        if ((status = apr_file_pipe_create(&attr->parent_err, &attr->child_err, 
                                   attr->pool)) != APR_SUCCESS) {
            return status;
        }
        switch (err) {
        case APR_FULL_BLOCK:
            break;
        case APR_PARENT_BLOCK:
            apr_file_pipe_timeout_set(attr->child_err, 0);
            break;
        case APR_CHILD_BLOCK:
            apr_file_pipe_timeout_set(attr->parent_err, 0);
            break;
        default:
            apr_file_pipe_timeout_set(attr->child_err, 0);
            apr_file_pipe_timeout_set(attr->parent_err, 0);
        }
    } 
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_child_in_set(apr_procattr_t *attr, apr_file_t *child_in,
                                   apr_file_t *parent_in)
{
    if (attr->child_in == NULL && attr->parent_in == NULL)
        apr_file_pipe_create(&attr->child_in, &attr->parent_in, attr->pool);

    if (child_in != NULL)
        apr_file_dup2(attr->child_in, child_in, attr->pool);

    if (parent_in != NULL)
        apr_file_dup2(attr->parent_in, parent_in, attr->pool);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_child_out_set(apr_procattr_t *attr, apr_file_t *child_out,
                                    apr_file_t *parent_out)
{
    if (attr->child_out == NULL && attr->parent_out == NULL)
        apr_file_pipe_create(&attr->child_out, &attr->parent_out, attr->pool);

    if (child_out != NULL)
        apr_file_dup2(attr->child_out, child_out, attr->pool);

    if (parent_out != NULL)
        apr_file_dup2(attr->parent_out, parent_out, attr->pool);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_child_err_set(apr_procattr_t *attr, apr_file_t *child_err,
                                   apr_file_t *parent_err)
{
    if (attr->child_err == NULL && attr->parent_err == NULL)
        apr_file_pipe_create(&attr->child_err, &attr->parent_err, attr->pool);

    if (child_err != NULL)
        apr_file_dup2(attr->child_err, child_err, attr->pool);

    if (parent_err != NULL)
        apr_file_dup2(attr->parent_err, parent_err, attr->pool);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_procattr_dir_set(apr_procattr_t *attr, 
                               const char *dir) 
{
    return apr_filepath_merge(&attr->currdir, NULL, dir, 
                              APR_FILEPATH_NATIVE, attr->pool);
}

APR_DECLARE(apr_status_t) apr_procattr_cmdtype_set(apr_procattr_t *attr,
                                     apr_cmdtype_e cmd) 
{
    if ((cmd != APR_PROGRAM) && (cmd != APR_PROGRAM_ENV))
        return APR_ENOTIMPL;
    attr->cmdtype = cmd;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_detach_set(apr_procattr_t *attr, apr_int32_t detach) 
{
    attr->detached = detach;
    return APR_SUCCESS;
}

#if APR_HAS_FORK
APR_DECLARE(apr_status_t) apr_proc_fork(apr_proc_t *proc, apr_pool_t *pool)
{
    int pid;
    
    if ((pid = fork()) < 0) {
        return errno;
    }
    else if (pid == 0) {
        proc->pid = pid;
        proc->in = NULL; 
        proc->out = NULL; 
        proc->err = NULL; 
        return APR_INCHILD;
    }
    proc->pid = pid;
    proc->in = NULL; 
    proc->out = NULL; 
    proc->err = NULL; 
    return APR_INPARENT;
}
#endif

static apr_status_t limit_proc(apr_procattr_t *attr)
{
#if APR_HAVE_STRUCT_RLIMIT && APR_HAVE_SETRLIMIT
#ifdef RLIMIT_CPU
    if (attr->limit_cpu != NULL) {
        if ((setrlimit(RLIMIT_CPU, attr->limit_cpu)) != 0) {
            return errno;
        }
    }
#endif
#ifdef RLIMIT_NPROC
    if (attr->limit_nproc != NULL) {
        if ((setrlimit(RLIMIT_NPROC, attr->limit_nproc)) != 0) {
            return errno;
        }
    }
#endif
#if defined(RLIMIT_AS)
    if (attr->limit_mem != NULL) {
        if ((setrlimit(RLIMIT_AS, attr->limit_mem)) != 0) {
            return errno;
        }
    }
#elif defined(RLIMIT_DATA)
    if (attr->limit_mem != NULL) {
        if ((setrlimit(RLIMIT_DATA, attr->limit_mem)) != 0) {
            return errno;
        }
    }
#elif defined(RLIMIT_VMEM)
    if (attr->limit_mem != NULL) {
        if ((setrlimit(RLIMIT_VMEM, attr->limit_mem)) != 0) {
            return errno;
        }
    }
#endif
#else
    /*
     * Maybe make a note in error_log that setrlimit isn't supported??
     */

#endif
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_child_errfn_set(apr_procattr_t *attr,
                                                       apr_child_errfn_t *errfn)
{
    /* won't ever be called on this platform, so don't save the function pointer */
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_error_check_set(apr_procattr_t *attr,
                                                       apr_int32_t chk)
{
    /* won't ever be used on this platform, so don't save the flag */
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_create(apr_proc_t *newproc,
									const char *progname, 
									const char * const *args, 
									const char * const *env,
                              		apr_procattr_t *attr, 
                              		apr_pool_t *pool)
{
	wiring_t		wire;
    int             addr_space;

    wire.infd  = attr->child_in ? attr->child_in->filedes : FD_UNUSED;
    wire.outfd = attr->child_out ? attr->child_out->filedes : FD_UNUSED;
    wire.errfd = attr->child_err ? attr->child_err->filedes : FD_UNUSED;

    newproc->in = attr->parent_in;
    newproc->out = attr->parent_out;
    newproc->err = attr->parent_err;

    /* attr->detached and PROC_DETACHED do not mean the same thing.  attr->detached means
     * start the NLM in a separate address space.  PROC_DETACHED means don't wait for the
     * NLM to unload by calling wait() or waitpid(), just clean up */
    addr_space = PROC_LOAD_SILENT | ((attr->cmdtype == APR_PROGRAM_ENV) ? 0 : PROC_CURRENT_SPACE);
    addr_space |= (attr->detached ? PROC_DETACHED : 0);

    if (attr->currdir) {
        char *fullpath = NULL;
        apr_status_t rv;

        if ((rv = apr_filepath_merge(&fullpath, attr->currdir, progname, 
                                     APR_FILEPATH_NATIVE, pool)) != APR_SUCCESS) {
            return rv;
        }
        progname = fullpath;
    } 

    if ((newproc->pid = procve(progname, addr_space, (const char**)env, &wire, 
        NULL, NULL, 0, NULL, (const char **)args)) == -1) {
        return errno;
    }

    if (attr->child_in) {
        apr_pool_cleanup_kill(apr_file_pool_get(attr->child_in), 
                              attr->child_in, apr_unix_file_cleanup);
        apr_file_close(attr->child_in);
    }
    if (attr->child_out) {
        apr_pool_cleanup_kill(apr_file_pool_get(attr->child_out), 
                              attr->child_out, apr_unix_file_cleanup);
        apr_file_close(attr->child_out);
    }
    if (attr->child_err) {
        apr_pool_cleanup_kill(apr_file_pool_get(attr->child_err), 
                              attr->child_err, apr_unix_file_cleanup);
        apr_file_close(attr->child_err);
    }


    apr_pool_cleanup_register(pool, (void *)newproc, apr_netware_proc_cleanup,
        apr_pool_cleanup_null);

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_wait_all_procs(apr_proc_t *proc,
                                                  int *exitcode,
                                                  apr_exit_why_e *exitwhy,
                                                  apr_wait_how_e waithow,
                                                  apr_pool_t *p)
{
    proc->pid = -1;
    return apr_proc_wait(proc, exitcode, exitwhy, waithow);
}

APR_DECLARE(apr_status_t) apr_proc_wait(apr_proc_t *proc,
                                        int *exitcode, apr_exit_why_e *exitwhy,
                                        apr_wait_how_e waithow)
{
    pid_t pstatus;
    int waitpid_options = WUNTRACED;
    int exit_int;
    int ignore;
    apr_exit_why_e ignorewhy;

    if (exitcode == NULL) {
        exitcode = &ignore;
    }

    if (exitwhy == NULL) {
        exitwhy = &ignorewhy;
    }

    if (waithow != APR_WAIT) {
        waitpid_options |= WNOHANG;
    }

    /* If the pid is 0 then the process was started detached. There 
       is no need to wait since there is nothing to wait for on a 
       detached process.  Starting a process as non-detached and
       then calling wait or waitpid could cause the thread to hang.
       The reason for this is because NetWare does not have a way 
       to kill or even signal a process to be killed.  Starting 
       all processes as detached avoids the possibility of a 
       thread hanging. */
    if (proc->pid == 0) {
        *exitwhy = APR_PROC_EXIT;
        *exitcode = 0;
        return APR_CHILD_DONE;
    }

    if ((pstatus = waitpid(proc->pid, &exit_int, waitpid_options)) > 0) {
        proc->pid = pstatus;

        if (WIFEXITED(exit_int)) {
            *exitwhy = APR_PROC_EXIT;
            *exitcode = WEXITSTATUS(exit_int);
        }
        else if (WIFSIGNALED(exit_int)) {
            *exitwhy = APR_PROC_SIGNAL;
            *exitcode = WTERMSIG(exit_int);
        }
        else {
            /* unexpected condition */
            return APR_EGENERAL;
        }

        return APR_CHILD_DONE;
    }
    else if (pstatus == 0) {
        return APR_CHILD_NOTDONE;
    }

    return errno;
}

APR_DECLARE(apr_status_t) apr_procattr_limit_set(apr_procattr_t *attr, apr_int32_t what, 
                          struct rlimit *limit)
{
    switch(what) {
        case APR_LIMIT_CPU:
#ifdef RLIMIT_CPU
            attr->limit_cpu = limit;
            break;
#else
            return APR_ENOTIMPL;
#endif
        case APR_LIMIT_MEM:
#if defined (RLIMIT_DATA) || defined (RLIMIT_VMEM) || defined(RLIMIT_AS)
            attr->limit_mem = limit;
            break;
#else
            return APR_ENOTIMPL;
#endif
        case APR_LIMIT_NPROC:
#ifdef RLIMIT_NPROC
            attr->limit_nproc = limit;
            break;
#else
            return APR_ENOTIMPL;
#endif
    }
    return APR_SUCCESS;
}  

