FROM debian:latest
ARG GIT_REPO

RUN \
    : "${GIT_REPO:?set GIT_REPO to the repo git url}"

RUN \
    echo "repo url to index: ${GIT_REPO}"

RUN \
  apt-get update && \
  apt-get -y install \
    python3 \
    python3-jinja2 \
    python3-pygments \
    python3-bsddb3 \
    exuberant-ctags \
    perl \
    git \
    apache2

RUN \
  git clone https://github.com/edahlgren/elixir.git /usr/local/elixir/

RUN \
  mkdir -p /srv/elixir-data/

RUN \
  mkdir -p /srv/elixir-data/generic/repo && \
  mkdir -p /srv/elixir-data/generic/data && \
  git clone "${GIT_REPO}" /srv/elixir-data/generic/repo/

ENV LXR_REPO_DIR /srv/elixir-data/generic/repo
ENV LXR_DATA_DIR /srv/elixir-data/generic/data

RUN \
  cd /usr/local/elixir/ && \
  ./update.py

# apache elixir config, see elixir README
# make apache less stricter about cgitb spam headers
COPY apache.conf /etc/apache2/sites-available/000-default.conf
COPY apache2.conf /etc/apache2/apache2.conf
RUN a2enmod cgi rewrite

EXPOSE 80

ENTRYPOINT ["/usr/sbin/apache2ctl", "-D", "FOREGROUND"]
