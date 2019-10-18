package main

import (
	"encoding/xml"
	"fmt"
	"log"
	"net/http"
)

type Respond struct {
	Name  string
	Value []string `xml:"Value>Value"`
}

func main() {
	http.HandleFunc("/", mainPage)

	port := ":2000"
	println("Server listen on port: ", port)
	err := http.ListenAndServe(port, nil)
	if err != nil {
		log.Fatal("ListenAndServe", err)
	}
}

func mainPage(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "text/xml")
	w.WriteHeader(200)
	r.ParseForm()

	for key, value := range r.Form {
		fmt.Printf("%s = %s \n", key, value)
	}
	text := r.FormValue("value")
	respond := Respond{"Value", []string{"value", text}}

	x, err := xml.MarshalIndent(respond, "", "  ")
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	w.Write(x)
}
