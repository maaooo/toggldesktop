package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"text/template"
)

const url = "https://support.toggl.com/sitemap/"

const ccTemplate = `
// Copyright 2015 Toggl Desktop developers.

// Do not modify contents. Content is generated by running
// go run src/script/update_help_artices.go

#include "../src/help_article.h"

#include "Poco/StringTokenizer.h"
#include "Poco/UTF8String.h"

namespace toggl {

HelpDatabase::HelpDatabase() {
{{range .}}  // NOLINT
    articles_.push_back(  // NOLINT
        HelpArticle(
            "{{ .Type }}",  // NOLINT
            "{{ .Name }}",  // NOLINT
            "{{ .URL }}",  // NOLINT
            "{{ .SearchText }}"));  // NOLINT
{{end}}  // NOLINT
}

std::vector<HelpArticle> HelpDatabase::GetArticles(
		const std::string keywords) {
    std::string lower = Poco::UTF8::toLower(keywords);
    Poco::StringTokenizer tokenizer(lower, ";, ",
        Poco::StringTokenizer::TOK_TRIM);
    std::vector<HelpArticle> result;
    for (std::vector<HelpArticle>::const_iterator it = articles_.begin();
            it != articles_.end();
            it++) {
        HelpArticle article = *it;
        for (Poco::StringTokenizer::Iterator sit = tokenizer.begin();
                sit != tokenizer.end();
                ++sit) {
            std::string keyword = *sit;
            if (article.SearchText.find(keyword) != std::string::npos) {
                result.push_back(article);
            }
        }
    }
    return result;
}

}   // namespace toggl
`

var fileName = filepath.Join("src", "help", "sitemap.json")

type Article struct {
	Type       string
	Name       string
	URL        string
	SearchText string
}

func main() {
	err := downloadArticles()

	if err == nil {
		err = generateSource()
	}

	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func generateSource() error {
	b, err := ioutil.ReadFile(fileName)
	if err != nil {
		return err
	}

	//fmt.Println("json", string(b))

	// sitemap map[6:[map[type:cat name:Team management url:team-management]
	var sitemap map[string][]map[string]string
	err = json.Unmarshal(b, &sitemap)
	if err != nil {
		return err
	}

	var articles []Article
	for number, list := range sitemap {
		fmt.Println("number", number)
		fmt.Println("articles", articles)

		for _, data := range list {
			var article Article
			article.Type = data["type"]
			article.Name = data["name"]
			article.URL = "https://support.toggl.com/" + data["url"]
			article.SearchText = strings.ToLower(article.Name)
			fmt.Println("article", article)

			articles = append(articles, article)
		}
	}

	//fmt.Println("sitemap", sitemap)

	t, err := template.New("cc").Parse(ccTemplate)
	if err != nil {
		return err
	}

	buf := bytes.NewBuffer(nil)
	err = t.Execute(buf, articles)
	if err != nil {
		return err
	}

	err = ioutil.WriteFile(filepath.Join("src", "help_article.cc"), buf.Bytes(), 0644)
	if err != nil {
		return err
	}

	return nil
}

func downloadArticles() error {
	response, err := http.Get(url)
	if err != nil {
		return err
	}
	defer response.Body.Close()

	os.Remove(fileName)

	output, err := os.Create(fileName)
	if err != nil {
		return err
	}
	defer output.Close()

	n, err := io.Copy(output, response.Body)
	if err != nil {
		return err
	}

	fmt.Println(n, "bytes downloaded.")

	return nil
}
