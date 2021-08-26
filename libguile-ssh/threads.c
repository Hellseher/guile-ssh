/* threads.c -- Initialization of SSH threads
 *
 * Copyright (C) 2013, 2020 Artyom V. Poptsov <poptsov.artyom@gmail.com>
 *
 * This file is part of Guile-SSH.
 *
 * Guile-SSH is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Guile-SSH is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Guile-SSH.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <libssh/callbacks.h>
#include "threads.h"

/* Since version 0.8.0, when libssh is dynamically linked, it is not necessary
   to call 'ssh_init'/'ssh_finalize' on systems which are fully supported with
   regards to threading (that is, system with pthreads available).

   See <https://api.libssh.org/stable/group__libssh.html> for details. */

#if ! HAVE_LIBSSH_0_8

/* Current SSH threading state */
static int pthreads_state = SSH_PTHREADS_DISABLED;

/* This procedure is called when the Guile-SSH library is unloaded. */
__attribute__((__destructor__)) void
cleanup ()
{
    ssh_finalize ();
}

/* Initialize threading if it has not been initialized yet. */
void
init_pthreads (void)
{
    if (pthreads_state == SSH_PTHREADS_DISABLED)
        {
            ssh_threads_set_callbacks (ssh_threads_get_pthread ());
            ssh_init ();
            pthreads_state = SSH_PTHREADS_ENABLED;
        }
}

#endif /* if ! HAVE_LIBSSH_0_8 */

/* threads.c ends here */
