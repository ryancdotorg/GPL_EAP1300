#senao_app_agent_date_sha

API_DOC:=senao_app_agent
API_DATE:= $(shell date +%Y_%m_%d_%H:%M)
GIT_LOG:=$(shell git log | head -n 1 | awk {'print $$2'} | cut -c 1-7)

all: add_version compile

compile: 
	@printf "compile ... "
	@aglio -i $(API_DOC).apib.tmp -o $(API_DOC).htm >/dev/null 2>&1
	@rm -f $(API_DOC).apib.tmp
	@printf "OK\n"
	
show:
	firefox $(API_DOC).htm

add_version:
	@echo "add version $(API_DATE)_$(GIT_LOG)"
	@cp $(API_DOC).apib $(API_DOC).apib.tmp
	@sed -i s/version_date_sha/$(API_DATE)_$(GIT_LOG)/g $(API_DOC).apib.tmp
