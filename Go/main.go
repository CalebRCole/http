package main

import (
	"log"
	"net/http"
	"os"
)

func main() {
	var root string

	if len(os.Args) > 1 {
		root = os.Args[1]
	} else {
		root = "."
	}

	fs := http.FileServer(http.Dir(root))

	http.Handle("/", fs)

	log.Println("Listening on :8080...")
	err := http.ListenAndServe(":8080", nil)
	if err != nil {
		log.Fatal(err)
	}
}
