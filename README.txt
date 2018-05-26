TODO: expliquer comment compiler et éxecuter dans plusieurs terminaux
expliquer

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Prerequisites
 * Installation
 * Configuration
 * Running the tests
 * Maintainers




INTRODUCTION
------------

Ce mini-projet a pour but d'implémenter un stockage clé-valeure avec des accès simultanés via TCP.
Ce stockage utilise un tableau associatif comme modèle.
Dans ce dernier, les données sont représentées comme une collection de pairs tel que chaque clé est unique dans la liste.
L'utilisation de sémaphore ou de mutex étudié en cours permet de pallier au problème de "race condition". 


PREREQUISTES
------------

Afin d'exécuter ce programme, vous avez besoin d'un compilateur C (tel que gcc par exemple) sur votre machine


INSTALLATION
------------

Dans un terminal, rendez-vous dans le dossier contenant les fichiers server.c et client.c.
Ensuite, lancez la commande:

  $ make

Afin de lancer le serveur, utilisez la commande:

  $ ./server

Afin de lancer le client, utilisez la commande:

  $ ./client


Le Key-Value Store implémenté permet d’ajouter, supprimer et modifier des entrées.
Vous pouvez écrire les commandes suivantes autant bien dans la fenêtre du server que celle du client. Les commandes sont interprètée par le serveur.
La structure de message acceptée par le serveur est la suivante:
• ’a vale’ pour ajouter une entrée avec une valeur mais sans préciser de clé. Dans ce cas, une clé est générée par le serveur qui la renvoie ensuite au client.
• ’ak key vale’ pour ajouter une entrée avec une valeur et une clé
• ’r key’ pour lire une entrée avec une certaine clé
• ’rv value’ pour lire une entrée avec une certaine value. Si plusieurs entrées ont la même valeur, seulement la première occurrence est renvoyées.
• ’d key’ pour supprimer une entrée avec une clé spécifique
• ’dv value’ pour supprimer une entrée avec une valeur spécifique. Si plusieurs entrées ont la valeur spécifiée, toutes les occurrences sont supprimèes.
• ’m key new value’ pour modifier la valeur d’une entrée avec une clé spécifique par la valeur ”new value”
• ’mv old vale new value’ pour modifier la valeur d’une entrée spécifique par une ”new value”. Si plusieurs entrées ont la même valeur, seulement la première occurrence est modifiée.
• ’p’ permet d’imprimer toutes les entrées du Key-Value Store.
• ’q’ permet de terminer la connexion avec le serveur et d’arrêter le client



CONFIGURATION
-------------

Aucune configuration spécifique n'est nécessaire.



RUNNING THE TESTS
-----------------

Quatre fichiers test peuvent être lancés à l'aide des commandes suivantes dans le terminal

  $ ./test1.sh
  $ ./test2.sh
  $ ./test3.sh
  $ ./test4.sh

Le test1 permet d'afficher....
Le test2 permet d'afficher....
Le test3 permet d'afficher....
Le test4 permet d'afficher....
