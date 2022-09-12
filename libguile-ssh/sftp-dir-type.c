/* sftp-dir-type.c -- SFTP dir type.
 *
 * Copyright (C) 2022 Artyom V. Poptsov <poptsov.artyom@gmail.com>
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

#include <libguile.h>
#include <libssh/libssh.h>

#include "common.h"
#include "error.h"
#include "sftp-session-type.h"
#include "sftp-dir-type.h"


static const char* GSSH_SFTP_DIR_TYPE_NAME = "sftp-dir";


scm_t_bits sftp_dir_tag;       /* Smob tag. */

gssh_sftp_dir_t*
make_gssh_sftp_dir ()
{
  return (gssh_sftp_dir_t *) scm_gc_malloc (sizeof (gssh_sftp_dir_t),
                                            GSSH_SFTP_DIR_TYPE_NAME);
}

SCM
gssh_sftp_dir_to_scm (sftp_dir dir, SCM path, SCM sftp_session)
{
  gssh_sftp_dir_t* data = make_gssh_sftp_dir ();
  SCM smob;
  data->gssh_sftp_session = sftp_session;
  data->dir = dir;
  data->path = path;
  SCM_NEWSMOB (smob, sftp_dir_tag, data);
  return smob;
}

gssh_sftp_dir_t *
gssh_sftp_dir_from_scm (SCM x)
{
  scm_assert_smob_type (sftp_dir_tag, x);
  return (gssh_sftp_dir_t *) SCM_SMOB_DATA (x);
}

SCM_GSSH_DEFINE (gssh_sftp_open_dir, "%gssh-sftp-open", 2,
                 (SCM sftp_session, SCM path))
#define FUNC_NAME s_gssh_sftp_open_dir
{
  gssh_sftp_session_t *sftp_sd = gssh_sftp_session_from_scm (sftp_session);
  sftp_dir dir;
  char* c_path;

  scm_dynwind_begin (0);

  c_path = scm_to_locale_string (path);
  scm_dynwind_free (c_path);

  dir = sftp_opendir (sftp_sd->sftp_session, c_path);
  if (dir == NULL)
    {
      guile_ssh_error1 (FUNC_NAME, "Could not open a directory",
                        scm_list_2 (sftp_session, path));
    }

  scm_dynwind_end ();

  return gssh_sftp_dir_to_scm (dir, path, sftp_session);
}
#undef FUNC_NAME



/* sftp-dir-type.c ends here. */
