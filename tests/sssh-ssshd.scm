;;; sssh-ssshd.scm -- Communication between sssh and ssshd.

;; Copyright (C) 2014, 2015 Artyom V. Poptsov <poptsov.artyom@gmail.com>
;;
;; This file is a part of Guile-SSH.
;;
;; Guile-SSH is free software: you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation, either version 3 of the
;; License, or (at your option) any later version.
;;
;; Guile-SSH is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with Guile-SSH.  If not, see <http://www.gnu.org/licenses/>.

(add-to-load-path (getenv "abs_top_srcdir"))

(use-modules (srfi srfi-64)
             (ice-9 popen)
             (ice-9 rdelim)
             (ice-9 regex)
             ;; Helper procedures
             (tests common))

(test-begin-with-log "sssh-ssshd")


;;;

(define *test-cmd* "uname -a")

(define *srv-address* INADDR_LOOPBACK)
(define *srv-port*    12600)
(define *srv-pid-file* "ssshd.pid")

(define *ssshd-cmd*
  (string-append
   %topbuilddir "/examples/ssshd.scm --detach"
   " --pid-file=" *srv-pid-file*
   " --port=" (number->string *srv-port*)
   " --rsakey=" %rsakey
   " --dsakey=" %dsakey))

(define *sssh-cmd*
  (string-append
   %topbuilddir "/examples/sssh.scm"
   " --identity-file=" %rsakey
   " --port=" (number->string *srv-port*)
   " --known-hosts-file=''"
   " " (inet-ntop AF_INET *srv-address*)
   " '" *test-cmd* "'"))

(setenv "GUILE_LOAD_PATH" (string-append %topdir "/modules"))

;; We must unset `SSH_AUTH_SOCK' to prevent sssh from asking SSH agent
;; (if it is present) for keys.
(unsetenv "SSH_AUTH_SOCK")

(define ssshd-pid #f)

(define (cleanup pid)
  (when pid
    (kill pid SIGTERM)
    (catch #t
      (lambda ()
        (waitpid pid))
      (const #t)))
  (and (file-exists? *srv-pid-file*)
       (delete-file *srv-pid-file*)))

(define (wait-pid-file max-tries pid-file)
  (let loop ((exists?    #f)
             (sleep-time 1)  ; s
             (try        1))
    (if exists?
        (let* ((p   (open-input-file pid-file))
               (pid (read-line p)))
          (string->number pid))
        (if (<= try max-tries)
            (begin
              (sleep sleep-time)
              (loop (file-exists? pid-file)
                    (1+ sleep-time)
                    (1+ try)))
            (begin
              (format #t "Couldn't read a PID file ~a in ~a tries.~%"
                      pid-file try)
              #f)))))


;;; Tests

(test-assert "ssshd, start"
  (let ((max-tries 10))
    (system *ssshd-cmd*)
    (let ((pid (wait-pid-file max-tries *srv-pid-file*)))
      (cleanup pid)
      pid)))

(test-assert "sssh, exec"
  (let ((max-tries 10))
    (system *ssshd-cmd*)
    (let* ((pid    (wait-pid-file max-tries *srv-pid-file*))
           (output (read-line (open-input-pipe *test-cmd*)))
           (p      (open-input-pipe *sssh-cmd*))
           (result (let r ((res "")
                           (l   (read-line p)))
                     (and (not (eof-object? l))
                          (if (string=? output res)
                              #t
                              (r (string-append res l)
                                 (read-line p)))))))
      (cleanup pid)
      result)))


(define exit-status (test-runner-fail-count (test-runner-current)))

(test-end "sssh-ssshd")

(exit (= 0 exit-status))

;;; sssh-ssshd.scm ends here.
