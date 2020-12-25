// Copyright 2020 Your Name <your_email>

// Реализация
// https://github.com/boostorg/beast/blob/develop/example/http/client/sync-ssl/http_client_sync_ssl.cpp
#ifndef INCLUDE_DOWNLOADER_HPP_
#define INCLUDE_DOWNLOADER_HPP_

#include <gumbo.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Parser.hpp"
#include "Queue.hpp"
#include "root_certificates.hpp"

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;  // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

class Downloader {
 public:
  explicit Downloader(unsigned poolsCount);
  void downloading(const std::string& url_, unsigned depth_, Parser& parser);

 private:
  std::string download_url(const std::string& host_,
                           const std::string& target_);
  std::string parse_url_to_host(std::string url);
  std::string parse_url_to_target(std::string url);
  void search_for_links(GumboNode* g_node);
  void download_next();
  std::map<std::string, std::string> urlss_;
  std::vector<std::future<std::string>> fut;
  ThreadPool pools;
};
//_________PUBLIC MEMBERS OF CLASS_________
Downloader::Downloader(unsigned int poolsCount) : pools(poolsCount) {}
// Начинаем скачивать страницы и обрабатывать их
void Downloader::downloading(const std::string& url_, unsigned depth_,
                             Parser& parser) {
  GumboOutput* out = gumbo_parse(
      download_url(parse_url_to_host(url_), parse_url_to_target(url_)).c_str());
  search_for_links(out->root);
  gumbo_destroy_output(&kGumboDefaultOptions, out);
  parser.parsing();
  while (depth_ > 0) {
    depth_--;
    download_next();
  }
}

//_________PRIVATE MEMBERS OF CLASS_________

std::string Downloader::download_url(const std::string& host_,
                                     const std::string& target_) {
  try {
    // Устанавливаем хост, порт и цель
    auto const host = host_.c_str();
    auto const port = "443";  // https://ru.wikipedia.org/wiki/HTTPS
    auto const target = target_.c_str();
    int version = 11;
    // io_context требуется для всех операций ввода-вывода
    boost::asio::io_context ioc;
    // Контекст SSL является обязательным и содержит сертификаты
    ssl::context ctx{ssl::context::sslv23_client};
    //Здесь хранится корневой сертификат, используемый для проверки
    load_root_certificates(ctx);
    // Эти объекты выполняют ввод-вывод
    tcp::resolver resolver{ioc};
    ssl::stream<tcp::socket> stream{ioc, ctx};
    // Установка имени хоста SNI (многим хостам это нужно для успешного
    // соединения)
    // https://ru.wikipedia.org/wiki/Server_Name_Indication
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
      boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                   boost::asio::error::get_ssl_category()};
      throw boost::system::system_error{ec};
    }
    // Просмотр доменного имени
    auto const results = resolver.resolve(host, port);
    // Соединение по IP-адресу, полученному из поиска
    boost::asio::connect(stream.next_layer(), results.begin(), results.end());
    // Выполнение SSL-соединения
    stream.handshake(ssl::stream_base::client);
    // Настройка сообщения HTTP GET-запроса
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // Отправление HTTP-запроса на удаленный хост
    http::write(stream, req);
    // Этот буфер используется для чтения и должен быть сохранен
    boost::beast::flat_buffer buffer;
    // Объявление контейнера для хранения ответа
    http::response<http::string_body> res;
    // Получение HTTP-ответа
    http::read(stream, buffer, res);
    // Корректное закрытие сокета
    boost::system::error_code ec;
    // stream.shutdown(ec);
    // Обработка исключений, возникающих при закрытии
    if (ec == boost::asio::error::eof) {
      ec.assign(0, ec.category());
    }
    if (ec) throw boost::system::system_error{ec};
    //        unsigned count;
    // Запись результата в очередб обработки
    queues_.push(res.body());

    return res.body();
  } catch (std::exception const& e) {
  }
  return "";
}
//Парсинг url-получение адреса хоста
std::string Downloader::parse_url_to_host(std::string url_) {
  if (url_.find("https://") == 0) url_ = url_.substr(8);
  std::string result_host;
  for (unsigned i = 0; i < url_.size(); ++i) {
    if ((url_[i] == '/') || (url_[i] == '?')) break;
    result_host += url_[i];
  }
  return result_host;
}
//Парсинг url-получение цели
std::string Downloader::parse_url_to_target(std::string url_) {
  if (url_.find("https:") == 0) url_ = url_.substr(8);
  std::string result_target;
  unsigned pos = 0;
  for (; pos < url_.size(); ++pos) {
    if ((url_[pos] == '/') || (url_[pos] == '?')) break;
  }
  for (unsigned i = pos; i < url_.size(); ++i) {
    result_target += url_[i];
  }

  return result_target;
}
// Обход ссылок и их последовательное скачивание
void Downloader::search_for_links(GumboNode* g_node) {
  if (g_node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href = nullptr;
  // Поиск ссылок и скачивание страниц
  if (g_node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&g_node->v.element.attributes, "href"))) {
    std::string curr_str = href->value;
    if (curr_str.find("https:") == 0) {
      unsigned count = urlss_.size();
      urlss_.insert(std::pair<std::string, std::string>(curr_str, "res"));
      if (urlss_.size() > count)
        fut.push_back(pools.enqueue(&Downloader::download_url, this,
                                    parse_url_to_host(curr_str),
                                    parse_url_to_target(curr_str)));
    }
  }
  GumboVector* children = &g_node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}
// Обработка следующего уровня ссылок
void Downloader::download_next() {
  unsigned counts = fut.size();
  for (unsigned i = 0; i < counts; ++i) {
    GumboOutput* out = gumbo_parse(fut[i].get().c_str());
    search_for_links(out->root);
    gumbo_destroy_output(&kGumboDefaultOptions, out);
  }
  std::cout << "finish" << std::endl;
}
#endif  // INCLUDE_DOWNLOADER_HPP_
