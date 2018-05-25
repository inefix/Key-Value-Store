TODO: expliquer comment compiler et éxecuter dans plusieurs terminaux 
expliquer 















Le Key-Value Store implémenté permet d’ajouter, supprimer et modifier des entrées. La structure de message acceptée par le serveur est la suivante:


• ’a vale’ pour ajouter une entrée avec une valeur mais sans préciser de clé. Dans ce cas, une clé est générée par le serveur qui la renvoie ensuite au client.
• ’ak key vale’ pour ajouter une entrée avec une valeur et une clé
• ’r key’ pour lire une entrée avec une certaine clé
• ’rv value’ pour lire une entrée avec une certaine value. Si plusieurs entrées ont la même valeur, seulement la première occurrence est renvoyées.
• ’d key’ pour supprimer une entrée avec une clé spécifique
• ’dv value’ pour supprimer une entrée avec une valeur spécifique. Si plusieurs entrées ont
la valeur spécifiée, toutes les occurrences sont supprimèes.
• ’m key new value’ pour modifier la valeur d’une entrée avec une clé spécifique par la valeur ”new value”
• ’mv old vale new value’ pour modifier la valeur d’une entrée spécifique par une ”new value”. Si plusieurs entrées ont la même valeur, seulement la première occurrence est modifiée.
• ’p’ permet d’imprimer toutes les entrées du Key-Value Store.
• ’q’ permet de terminer la connexion avec le serveur et d’arrêter le client

Le serveur accepte autant bien les commandes si dessus si elles sont envoyées par un client que si elles sont écrites directement dans la sa fenêtre.
