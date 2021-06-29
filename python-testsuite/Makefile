
source: get_git_version get_news_version
	@echo "Creating an archive"
	@echo NEWS version is: $(NEWS_VERSION)
	@echo GIT hash is: $(GIT_SHORT_SHA1)
	git archive -o enyx-oe-hwstrat-hls-demo-python-testsuite-$(NEWS_VERSION)-git$(GIT_SHORT_SHA1)-Source.tar.gz  --prefix enyx-oe-hwstrat-hls-demo-python-testsuite/ --format tar.gz HEAD

get_git_version:
	@echo Extract GIT short hash
	$(eval GIT_SHORT_SHA1 := $(shell git rev-parse --short HEAD))

get_news_version:
	@echo Extract NEWS version
	$(eval NEWS_VERSION := $(shell ./extract_version_from_news_file.sh))

get_git_branch:
	@echo Extract branch version
	$(eval GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD))

upload: get_git_branch source
	$(eval ARCHIVE_FILENAME := enyx-oe-hwstrat-hls-demo-python-testsuite-$(NEWS_VERSION)-git$(GIT_SHORT_SHA1)-Source.tar.gz)
	@echo "Manual upload of ${ARCHIVE_FILENAME} for branch ${GIT_BRANCH}"
	nxa-upload -f ${ARCHIVE_FILENAME} -v ${NEWS_VERSION} -r integration-trading -p enyx-oe-hwstrat-hls-demo-python-testsuite -a source -c trading-demo -b ${GIT_BRANCH}

clean:
	@echo Cleaning old archive
	rm -f *.tar.gz
