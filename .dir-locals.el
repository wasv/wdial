((nil . ((eval . (let ((root (projectile-project-root)))
                   (setq-local company-clang-arguments
                               (list (concat "-I" root "include")
                                     (concat "-I" root "libopencm3/include")
                                     "-DSTM32F1"))
                   (setq-local flycheck-clang-include-path
                               (list (concat root "include")
                                     (concat root "libopencm3/include")))
                   (setq-local flycheck-clang-definitions
                               (list (concat  "STM32F1"))))))))
