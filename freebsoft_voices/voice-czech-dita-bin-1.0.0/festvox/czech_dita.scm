(require 'czech-unisyn)

(defvar czech-dita-unisyn-index "/usr/share/festival/voices/czech/czech_dita/group/dita.group")

(defvar czech-dita-int-params '((f0_mean 222) (f0_std 15)))

(set! czech-insert-filling-vowels nil)

(czech-proclaim-voice (dita (gender female)) ""
  (czech-unisyn-init 'czech_dita czech-dita-unisyn-index)
  (set! czech-int-simple-params* czech-dita-int-params))

(provide 'czech_dita)
