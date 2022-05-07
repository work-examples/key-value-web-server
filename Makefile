# =================================
CODE_DIR_ROOT = ./

FLAKE8_CONFIG       = ./.config/.flake8
MARKDOWNLINT_CONFIG = ./.config/.markdownlint.yaml
YAMLLINT_CONFIG     = ./.config/.yamllint.yaml

# =================================

check-all: check-md check-yaml check-pep8

# =================================

check-md:
# Using two slashes at the beginning of the paths for Windows bash shell
	docker run --rm --tty --network=none --volume="${CURDIR}:/markdown:ro" \
		--workdir=//markdown/ \
		06kellyjac/markdownlint-cli:0.27.1-alpine \
            --config=${MARKDOWNLINT_CONFIG} \
            --ignore=./dependency/CrowCpp/README.md \
			-- ./

check-yaml:
# Using two slashes at the beginning of the paths for Windows bash shell
	docker run --rm --tty --network=none --volume="$(CURDIR):/data:ro" \
		--workdir=//data/ \
		cytopia/yamllint:1.26-0.9 \
            -c=${YAMLLINT_CONFIG} \
			--strict \
			-- ./

check-pep8:
# Using two slashes at the beginning of the paths for Windows bash shell
	docker run --rm --tty --network=none --volume="${CURDIR}:/apps:ro" \
		--workdir=//apps/ \
		alpine/flake8:4.0.1 \
			--config=${FLAKE8_CONFIG} \
			-- ${CODE_DIR_ROOT}

# =================================
